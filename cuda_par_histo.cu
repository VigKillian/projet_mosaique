#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#include <float.h>
#include <cuda_runtime.h>
#define NB_BASE_DE_DONNEE 10000
using namespace std;
typedef unsigned char OCTET;

__global__ void computeHistoKernel(
    OCTET* d_imgIn,
    int* d_histos,
    int nW,
    int nH,
    int tailleBloc,
    int nbImagettes
){


}

__global__ void matchImagetteKernel(
  OCTET* d_imgIn,
  float* d_moyennesImagettes,
  int* d_resultImagetteIds,
  int nW,
  int nH,
  int tailleBloc,
  int nbImagettes
){
  int tileX = blockIdx.x*blockDim.x+threadIdx.x;
  int tileY = blockIdx.y*blockDim.y+threadIdx.y;

  int nbTileX = nW/tailleBloc;
  int nbTileY = nH/tailleBloc;

  if (tileX >= nbTileX || tileY >= nbTileY) return;

  int startX = tileX * tailleBloc;
  int startY = tileY * tailleBloc;

  float sum = 0.f;
  for(int dy = 0;dy<tailleBloc;dy++){
    for(int dx = 0;dx<tailleBloc;dx++){
      int px = startX + dx;
      int py = startY + dy;
      sum += d_imgIn[py*nW+px];
    }
  }
  sum /= (tailleBloc*tailleBloc);
  float minDiff = FLT_MAX;
  int bestId = -1;
  for (int i = 0; i < nbImagettes; i++) {
    float diff = fabsf(d_moyennesImagettes[i] - sum);
    if (diff < minDiff) {
      minDiff = diff;
      bestId = i;
    }
  }

  d_resultImagetteIds[tileY * nbTileX + tileX] = bestId;

}

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, tailleBloc;
  float moyennesImagettes[NB_BASE_DE_DONNEE];

  if (argc != 4) 
    {
      printf("Usage: ImageIn.pgm ImageOut.pgm taille_bloc \n"); 
      exit (1);
    }

  std::chrono::time_point<std::chrono::high_resolution_clock> _t0 = std::chrono::high_resolution_clock::now();
  
  sscanf (argv[1],"%s",cNomImgLue);
  sscanf (argv[2],"%s",cNomImgEcrite);
  sscanf (argv[3],"%d",&tailleBloc);

  OCTET *ImgIn, *ImgOut;

  lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;

  allocation_tableau(ImgIn, OCTET, nTaille);
  lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
  allocation_tableau(ImgOut, OCTET, nTaille);

  // Charger les imagettes et calculer leur moyenne de luminosité
  for(int idImagette = 0; idImagette < NB_BASE_DE_DONNEE; idImagette++){
    OCTET *ImgIn_imagette;
    int nH_imagette, nW_imagette, nTaille_imagette;
    
    loadImagette(idImagette+1, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    
    float moyenneImagette = 0.f;
    for(int j = 0; j < nTaille_imagette; j++){
      moyenneImagette += ImgIn_imagette[j];
    }
    moyenneImagette /= float(nTaille_imagette);
    moyennesImagettes[idImagette] = moyenneImagette;
    
    free(ImgIn_imagette);
  }


  // Remplacement des blocs de l'image originale
  OCTET* d_imgIn;
  float* d_moyennesImagettes;
  int* d_resultImagetteIds;
  int nbTile = nH*nW/tailleBloc/tailleBloc;
  cudaMalloc(&d_imgIn,sizeof(OCTET)*nTaille);
  cudaMalloc(&d_moyennesImagettes,sizeof(float)*NB_BASE_DE_DONNEE);
  cudaMalloc(&d_resultImagetteIds,sizeof(int)*nbTile);
  cudaMemcpy(d_imgIn, ImgIn, sizeof(OCTET) * nTaille, cudaMemcpyHostToDevice);
  cudaMemcpy(d_moyennesImagettes,moyennesImagettes,sizeof(float)*NB_BASE_DE_DONNEE,cudaMemcpyHostToDevice);


  dim3 threadsPerBlock(16,16);
  dim3 nbBlock((nW/tailleBloc+15)/16,(nH/tailleBloc+15)/16);
  matchImagetteKernel<<<nbBlock,threadsPerBlock>>>(
    d_imgIn,d_moyennesImagettes,d_resultImagetteIds,nW,nH,tailleBloc,NB_BASE_DE_DONNEE
  );
  cudaDeviceSynchronize(); //attend que toutes les operations soient terminees.
  cudaError_t err = cudaGetLastError();
  if (err != cudaSuccess) {
    std::cerr << "CUDA kernel launch failed: " << cudaGetErrorString(err) << std::endl;
  }
  // copy back to host
  int* resultImagetteIds = new int[nbTile];
  cudaMemcpy(resultImagetteIds,d_resultImagetteIds,sizeof(int)*nbTile,cudaMemcpyDeviceToHost);

  for(int i = 0; i <= nH - tailleBloc; i += tailleBloc){
    for(int j = 0; j <= nW - tailleBloc; j += tailleBloc){
      int pixelDepart = i * nW + j;
      OCTET *ImgOut_imagette;
      OCTET *ImgIn_imagette;
      int nH_imagette, nW_imagette, nTaille_imagette;
      
      loadImagette(resultImagetteIds[i/tailleBloc*nW/tailleBloc+j/tailleBloc]+1, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
      allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc);
      resize_imagette(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
      
      // Remplacer les pixels du bloc par ceux de l'imagette
      for(int k = 0; k < tailleBloc; k++){
        for(int p = 0; p < tailleBloc; p++){
          ImgOut[pixelDepart + k * nW + p] = ImgOut_imagette[k * tailleBloc + p];
        }
      }
      
      free(ImgIn_imagette);
      free(ImgOut_imagette);
    }
  }

  ecrire_image_pgm(cNomImgEcrite, ImgOut, nH, nW);
  

  std::chrono::time_point<std::chrono::high_resolution_clock> _t1 = std::chrono::high_resolution_clock::now();
  std::cout << "Photo-mosaïque produite en : "<<std::chrono::duration<double>(_t1-_t0).count() << "s" << std::endl;

  std::cout<<"PSNR par rapport à l'image d'entrée : "<<calculer_PSNR(ImgIn, ImgOut, nTaille)<<"dB"<<std::endl;
  free(ImgIn);
  free(ImgOut);

  return 1;
}