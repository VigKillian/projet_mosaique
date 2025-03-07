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
    int nH_tile,nW_tile,nTaille_tile;
    loadImagette(i,ImgIn_tile,nH_tile,nW_tile,nTaille_tile);
    float sum_tile = 0.f;
    for(int j=0;j<nTaille_tile;j++){
      sum_tile += ImgIn_tile[j];
    }
    sum_tile /=float(nTaille_tile);
    imagettes.push_back({i,sum_tile,0}); // charger information : ID: i , moyen: sum , isUsed: not
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

      float diff_min = FLT_MAX;
      int best_imagette_id = -1;

      for(Imagette &imagette : imagettes){
          float diff = fabs(imagette.moyen-sum);
          if(diff<diff_min && !imagette.isUsed){
            diff_min = diff;
            best_imagette_id = imagette.ID;
            // je peux pas mettre a jour isUsed ici! si non avant trouver le diff_min je vais peut-etre mettre tous imagette.isUsed a 1!
            //imagette.isUsed = 1; 
        }
      }
      // faut mettre a jour ici
      for(Imagette &imagette : imagettes){
        if(imagette.ID == best_imagette_id){
          imagette.isUsed = 1;
          break; 
        }
      }
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