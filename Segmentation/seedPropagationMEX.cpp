/***************************************************************
 * Propagation of Seeds
 * using Relative Intensity Changes
 *
 * C. Kirst, The Rockefeller University 2014
 *
 * propagate seeds to prevent oversegmentation
 *
 ***************************************************************/

#include <math.h>
#include "mex.h"
#include <queue>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

/* Input Arguments */
#define IM_IN              prhs[0]
#define SEEDS_IN           prhs[1]
#define MASK_IN            prhs[2]
#define LAMBDA_IN          prhs[3]
#define CUTOFF_DIST_IN     prhs[4]
#define RADIUS_IN          prhs[5]
#define INTENSITY_REF_IN   prhs[6]


/* Output Arguments */
#define SEEDS_OUT         plhs[0]
#define DISTANCES_OUT     plhs[1]

#define IJ(i,j) ((j)*m + (i))


class Pixel { 
public:
   double distance;
   double spatial_distance;
   unsigned int i, j;
   double label;
   
public:
   Pixel (double d, double sd, unsigned int ini, unsigned int inj, double l) : 
          distance(d), spatial_distance(sd), i(ini), j(inj), label(l) {
   }
          
public:   
   
   
   
};

struct Pixel_compare { 
   bool operator() (const Pixel& a, const Pixel& b) const { 
      return a.distance > b.distance; 
   }
};

typedef priority_queue<Pixel, vector<Pixel>, Pixel_compare> PixelQueue;

static double clamped_fetch(double *image, int i, int j, int m, int n) {
  if (i < 0) i = 0;
  if (i >= m) i = m-1;
  
  if (j < 0) j = 0;
  if (j >= n) j = n-1;

  return (image[IJ(i,j)]);
};


/* average intensity around a point */
static double average_intensity(double * image, 
                                int i,  int j, 
                                unsigned int m, unsigned int n,  int radius) {
   int delta_i, delta_j;
   double intensity = 0.0;

   // Calculate average pixel intensities
   for (delta_j = -radius; delta_j <= radius; delta_j++) {
      for (delta_i = -radius; delta_i <= radius; delta_i++) {
            intensity += clamped_fetch(image, i + delta_i, j + delta_j, m, n);
      }
   }
    
   double norm = 2*radius+1.0;
   return (intensity/(norm*norm*norm));
};


/* difference between two pixels */
static double difference(double *image,
           int i1,  int j1,
           int i2,  int j2,
           unsigned int m, unsigned int n,
           int radius, double ref_intensity,  double lambda, double spatial_dist, double& space_dist)
{
   int delta_i, delta_j;
   double pixel_diff = 0.0;

   // Calculate average pixel difference
   for (delta_j = -radius; delta_j <= radius; delta_j++) {
      for (delta_i = -radius; delta_i <= radius; delta_i++) {

            //pixel_diff += fabs(clamped_fetch(image, i1 + delta_i, j1 + delta_j, k1 + delta_k, m, n, l) - 
            //                   clamped_fetch(image, i2 + delta_i, j2 + delta_j, k2 + delta_k, m, n, l));    
            pixel_diff += fabs(clamped_fetch(image, i2 + delta_i, j2 + delta_j, m, n) - ref_intensity);           

      }
   }
   
   double norm = (2*radius+1.0);
   pixel_diff *= 1.0/(norm*norm*norm);
 
   // distance (is 'semi geodesic') 
   double d1 = i1 - i2; double d2 = j1 -j2;
   space_dist = sqrt(d1*d1 + d2*d2) + spatial_dist;  
  
   //here is space for taking into account gradient image / gradient crossings form the label center to the new pixel etc....
  
   return ((1 - lambda) * pixel_diff + lambda * space_dist);
};




static void push_neighbors_on_queue(PixelQueue &pq, /*double dist,*/ double spatial_dist,
                        double *image,
                        unsigned int i, unsigned int j,
                        unsigned int m, unsigned int n,
                        int radius, double ref_intensity,  double lambda, double cutoff_dist,
                        double label, double *seeds_out, mxLogical* mask_in)
{
   double d, sd;
  
   /* 26-connected */
   int di, dj, idi, jdj;
   for(di = -1; di <= 1; di++) {
      for (dj = -1; dj <= 1; dj++) {  
            idi = i +di; jdj = j + dj;
            if (idi>=0 && idi < m && jdj >= 0 && jdj < n &&
               mask_in[IJ(idi,jdj)] &&  0 == seeds_out[IJ(idi,jdj)]) {
               //only push if neighbours are within resonable distance
               d = difference(image, i, j, idi, jdj, m, n, radius, ref_intensity, lambda, spatial_dist, sd);
               //cout << "i,j,k=" << i << "," << j << "," << k << endl;
               //cout << "di,j,k=" << idi << "," << jdj << "," << kdk << endl;
               //cout << "d=" << d << " sp_dist=" << spatial_dist << " sd=" << sd << endl;
               //cout << "ref_intensity=" << ref_intensity << endl;
               if (d < cutoff_dist) {
                  pq.push(Pixel(d, sd, idi, jdj, label));
               }
            }
      }
   }          
};

static void propagate(double *seeds_in, double *im_in, mxLogical *mask_in, 
                      double *seeds_out, 
                      double *dists,
                      unsigned int m, unsigned int n,
                      int radius, 
                      double *center_intensities, unsigned int nlabel,
                      double lambda, double cutoff_distance) {

   unsigned int i, j;
  
   PixelQueue pixel_queue;
   //map<double, double> center_intensities;  / for auto center_intensities 
  
  
   /* initialize dist to Inf, read seeds_in and wrtite out to seeds_out */
   for (j = 0; j < n; j++) {
      for (i = 0; i < m; i++) {
            dists[IJ(i,j)] = mxGetInf();            
            seeds_out[IJ(i,j)] = seeds_in[IJ(i,j)];
      }
   }
  
   /* if the pixel is already labeled (i.e, labeled in seeds_in) and within a mask, 
    * then set dist to 0 and push its neighbors for propagation */
   for (j = 0; j < n; j++) {
      for (i = 0; i < m; i++) { 
            double label = seeds_in[IJ(i,j)];
            if ((label > 0) && (mask_in[IJ(i,j)])) {

               if ((int) label >= nlabel || ( (int) label < 0) || (fabs(label - (int) label) > 0) ) {
                  //cout << label << " " << (int) label << " " << fabs(label - (int) label) << " "<< nlabel << endl;
                  //cout << ((int) label >= nlabel) << " " << ((int) label < 0) << " " <<  (fabs(label - (int) label) > 0) << endl;

                  mexErrMsgTxt("Inconsistent label of seeds, should be integer from 1 - nlabel, 0 for background %g.");
               };
          
               dists[IJ(i,j)] = 0.0;
            
        
               /* auto initialize intensities */
               /*
               double norm = 1.0;
               if (radius_center >= 0) {
                  norm = average_intensity(im_in, i, j, m, n, radius_center);
               }
               center_intensities[label] = norm;
               */
            
               push_neighbors_on_queue(pixel_queue,/* 0.0,*/ 0.0, im_in, i, j, m, n, radius, center_intensities[(int) label], lambda, cutoff_distance, label, seeds_out, mask_in);
        
            }
      }   
   }

   while (! pixel_queue.empty()) {
      Pixel p = pixel_queue.top();
      pixel_queue.pop();
      //cout << "popped " << p.i << " " << p.j << " " << p.k << endl;

      if ((dists[IJ(p.i, p.j)] > p.distance) /* && (mask_in[IJ(p.i,p.j, p.k)])*/) {
         dists[IJ(p.i, p.j)] = p.distance;
         seeds_out[IJ(p.i, p.j)] = p.label;      
         push_neighbors_on_queue(pixel_queue, /* p.distance,*/ p.spatial_distance, im_in, p.i, p.j, m, n,
                                 radius, center_intensities[(int) p.label], lambda, cutoff_distance, p.label, seeds_out, mask_in);
      }
   }
}

void mexFunction( int nlhs, mxArray *plhs[], 
                  int nrhs, const mxArray*prhs[] ) { 
   double *seeds_in, *im_in; 
   mxLogical *mask_in;
   double *seeds_out, *dists;
   double *lambda;
   double *cutoff_distance;
   int radius /*, radius_center*/;    
   unsigned int m, n; 
   
   double *intensity_refs;
   unsigned int nlabel;
   mwSize  ndim;
    
    
   /* Check for proper number of arguments */  
   if (nrhs != 7) { 
      mexErrMsgTxt("7 input arguments required."); 
   } else if (nlhs !=1 && nlhs !=2) {
      mexErrMsgTxt("The number of output arguments should be 1-2."); 
   } 
    
   /* Size of Image */
   ndim = mxGetNumberOfDimensions(IM_IN);
   if (2 != ndim){
      mexErrMsgTxt("input image must be 2d grayscale image");
   }
   
   const mwSize* size = mxGetDimensions(IM_IN);
   m = size[0];
   n = size[1];
   
   //cout << m << " " << n << " " << l << endl;

   /* Size of Labeled Image */
   ndim = mxGetNumberOfDimensions(SEEDS_IN);
   size = mxGetDimensions(SEEDS_IN);
   
   /* Check for the correct number of indices  */
   if (2 != ndim){
      mexErrMsgTxt("input label image must be 3d grayscale image");
   }

   if ((m != size[0]) || (n != size[1])) {
      mexErrMsgTxt("sizes of image and labels do not agree");
   }

   if (! mxIsDouble(IM_IN)) {
      mexErrMsgTxt("First argument must be a double array.");
   }
   if (! mxIsDouble(SEEDS_IN)) {
      mexErrMsgTxt("Second argument must be a double array.");
   }
   if (! mxIsLogical(MASK_IN)) {
      mexErrMsgTxt("Third argument must be a logical array.");
   }

   /* Create matrices for the return arguments */    
    
   SEEDS_OUT = mxCreateNumericArray(ndim, size, mxDOUBLE_CLASS, mxREAL); 
   DISTANCES_OUT = mxCreateNumericArray(ndim, size, mxDOUBLE_CLASS, mxREAL);
    
   /* Assign pointers to the various parameters */ 
   seeds_in = mxGetPr(SEEDS_IN);
   im_in = mxGetPr(IM_IN);
   mask_in = mxGetLogicals(MASK_IN);
   lambda = mxGetPr(LAMBDA_IN);
   cutoff_distance = mxGetPr(CUTOFF_DIST_IN);
   intensity_refs = mxGetPr(INTENSITY_REF_IN);
   nlabel = mxGetM(INTENSITY_REF_IN);

   double * dptr = mxGetPr(RADIUS_IN);
   radius = (int) (*dptr);
   //dptr = mxGetPr(RADIUS_CENTER_IN);
   //radius_center = (int) (*dptr);
    
   seeds_out = mxGetPr(SEEDS_OUT);
   dists = mxGetPr(DISTANCES_OUT);

   //cout << "lambda = " << *lambda << " cutoff_difference = " << *cutoff_distance << endl;
   //cout << "radius = " << radius << " nlabel=" << nlabel << " (m,n,l)=(" << m << "," << n << "," << l << ")" << endl;
   
   /* Do the actual computations in a subroutine */
   propagate(seeds_in, im_in, mask_in, seeds_out, dists, m, n, radius, intensity_refs, nlabel, *lambda, *cutoff_distance); 

   if (nlhs == 1) {
      mxDestroyArray(DISTANCES_OUT);
   }      

   return;
};
