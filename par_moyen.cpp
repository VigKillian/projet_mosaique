// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
using namespace std;

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, taile_tile;
  float moy_tile[10000];

  if (argc != 4) 
    {
      printf("Usage: ImageIn.pgm ImageOut.pgm taille_tile \n"); 
      exit (1) ;
    }

  sscanf (argv[1],"%s",cNomImgLue) ;
  sscanf (argv[2],"%s",cNomImgEcrite);
  sscanf (argv[3],"%d",&taile_tile);

  OCTET *ImgIn, *ImgOut;

  lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;

  allocation_tableau(ImgIn, OCTET, nTaille);
  lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
  allocation_tableau(ImgOut, OCTET, nTaille);

  //lire biblio des images
  
  for(int i=1;i<=1000;i++){
    int nH_tile,nW_tile,nTaille_tile;
    OCTET *ImgIn_tile;
    string nom_file = "./img_tile/"+std::to_string(i)+".pgm";
    lire_nb_lignes_colonnes_image_pgm((char*)nom_file.c_str(),&nH_tile,&nW_tile);
    nTaille_tile = nW_tile*nH_tile;
    allocation_tableau(ImgIn_tile, OCTET, nTaille_tile);
    lire_image_pgm((char*)nom_file.c_str(), ImgIn_tile, nTaille_tile);
    float sum_tile = 0.f;
    for(int j=0;j<nTaille_tile;j++){
      sum_tile += ImgIn_tile[j];
    }
    sum_tile /=float(nTaille_tile);
    moy_tile[i] = sum_tile;
    free(ImgIn_tile);
  }


// Partie l'image originale
  for(int i=0;i<=nH-taile_tile;i+=taile_tile){
    for(int j=0;j<=nW-taile_tile;j+=taile_tile){
      int pixel_0 = i*nW+j;
      float sum = 0.f;
      for(int k=0;k<taile_tile;k++){
        for(int p=0;p<taile_tile;p++){
          sum += ImgIn[pixel_0+k*nW+p];
        }
      }
      sum/=(float)(taile_tile*taile_tile);
      for(int k=0;k<taile_tile;k++){
        for(int p=0;p<taile_tile;p++){
          ImgOut[pixel_0+k*nW+p] = round(sum);
        }
      }
    }
  }


  ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
  free(ImgIn); free(ImgOut);

  return 1;
}