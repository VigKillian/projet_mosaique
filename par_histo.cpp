// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <cmath>
#include <float.h>
#define NB_BASE_DE_DONNEE 10000
using namespace std;


int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, taile_tile;
  // float moy_tile[NB_BASE_DE_DONNEE][2]; 
  std::vector<Imagette> imagettes;

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
  
  for(int i=1;i<=NB_BASE_DE_DONNEE;i++){
    OCTET *ImgIn_tile;
    std::array<int,256> histo{0};
    int nH_tile,nW_tile,nTaille_tile;
    loadImagette(i,ImgIn_tile,nH_tile,nW_tile,nTaille_tile);
    for(int j = 0;j<nTaille_tile;j++){
      histo[ImgIn_tile[j]]++;
    }

    imagettes.push_back({i,0.f,histo,0}); // charger information : ID: i , moyen: sum , isUsed: not
    free(ImgIn_tile);
  }


  // Partie l'image originale
  for(int i=0;i<=nH-taile_tile;i+=taile_tile){
    for(int j=0;j<=nW-taile_tile;j+=taile_tile){
      int pixel_0 = i*nW+j;
      std::array<int,256> histo_local{0};      
      for(int k=0;k<taile_tile;k++){
        for(int p=0;p<taile_tile;p++){
          // sum += ImgIn[pixel_0+k*nW+p];
          histo_local[ImgIn[pixel_0+k*nW+p]]++;
        }
      }

      float min = FLT_MAX;
      int best_imagette_id = -1;

      for(Imagette &imagette : imagettes){
        float res = 0.f;
        for(int cou=0;cou<256;cou++){
          res +=(float)sqrt(histo_local[cou]*imagette.histo[cou]);
        }
        res = -log(res);
        if(res<min){
          min = res;
          best_imagette_id = imagette.ID;
        }
      }
      // faut mettre a jour ici
      // for(Imagette &imagette : imagettes){
      //   if(imagette.ID == best_imagette_id){
      //     imagette.isUsed = 1;
      //     break; 
      //   }
      // }
      OCTET *ImgOut_imagette;
      OCTET *ImgIn_tile;
      int nH_tile,nW_tile,nTaille_tile;
      loadImagette(best_imagette_id,ImgIn_tile,nH_tile,nW_tile,nTaille_tile);
      allocation_tableau(ImgOut_imagette, OCTET, taile_tile*taile_tile);
      resize_imagette(ImgIn_tile,nH_tile,nW_tile,ImgOut_imagette,taile_tile,taile_tile);
      
      //  remplacer les pixels de bloc par les pixels d'imagettes
      for(int k=0;k<taile_tile;k++){
        for(int p=0;p<taile_tile;p++){
          ImgOut[pixel_0+k*nW+p] = ImgOut_imagette[k*taile_tile+p];
        }
      }
      free(ImgIn_tile);free(ImgOut_imagette);
    }
  }


  ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
  free(ImgIn); free(ImgOut);

  return 1;
}