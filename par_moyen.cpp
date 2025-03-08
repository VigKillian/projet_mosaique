#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#include <float.h>
#define NB_BASE_DE_DONNEE 10000
using namespace std;

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
  for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++){
    OCTET *ImgIn_imagette;
    int nH_imagette, nW_imagette, nTaille_imagette;
    
    loadImagette(idImagette, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    
    float moyenneImagette = 0.f;
    for(int j = 0; j < nTaille_imagette; j++){
      moyenneImagette += ImgIn_imagette[j];
    }
    moyenneImagette /= float(nTaille_imagette);
    moyennesImagettes[idImagette] = moyenneImagette;

    free(ImgIn_imagette);
  }

  // Remplacement des blocs de l'image originale
  for(int i = 0; i <= nH - tailleBloc; i += tailleBloc){
    for(int j = 0; j <= nW - tailleBloc; j += tailleBloc){
      int pixelDepart = i * nW + j;
      float moyenneBloc = 0.f;

      for(int k = 0; k < tailleBloc; k++){
        for(int p = 0; p < tailleBloc; p++){
          moyenneBloc += ImgIn[pixelDepart + k * nW + p];
        }
      }
      moyenneBloc /= float(tailleBloc * tailleBloc);

      float differenceMin = FLT_MAX;
      int best_imagette_id = -1;

      for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++)
      {
        float differenceActuelle = fabs(moyennesImagettes[idImagette] - moyenneBloc);
        if(differenceActuelle < differenceMin){
          differenceMin = differenceActuelle;
          best_imagette_id = idImagette;
        }
      }

      // Charger l'imagette sélectionnée et la réduire si nécessaire
      OCTET *ImgOut_imagette;
      OCTET *ImgIn_imagette;
      int nH_imagette, nW_imagette, nTaille_imagette;
      
      loadImagette(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
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
  free(ImgIn);
  free(ImgOut);

  return 1;
}
