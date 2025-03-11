// test_couleur.cpp : Seuille une image en niveau de gris
#define NB_BASE_DE_DONNEE 10000
#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#include <float.h>

using namespace std;

int main(int argc, char *argv[]) {
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille ,nR,nG,nB;
  std::vector<ImagetteCouleur> listeImagettes;
  int tailleBloc;


  if (argc != 3) {
    printf("Usage: ImageIn.ppm mageOut.ppm taille_bloc \n");
    exit(1);
  }

  sscanf(argv[1], "%s", cNomImgLue);
  sscanf(argv[2], "%s", cNomImgEcrite);
  sscanf (argv[3],"%d",&tailleBloc);




  OCTET *ImgIn,*ImgOut;

  lire_nb_lignes_colonnes_image_ppm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;

  int nTaille3 = nTaille * 3;
  allocation_tableau(ImgIn, OCTET, nTaille3);
  lire_image_ppm(cNomImgLue, ImgIn, nH * nW);
  allocation_tableau(ImgOut, OCTET, nTaille3);

  // Charger les imagettes et calculer leur moyenne de luminosité
  for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++){
    OCTET *ImgIn_imagette, *ImgOut_imagette;
    int nH_imagette, nW_imagette, nTaille_imagette;
    vector<float> moyen = 
    loadImagette_cou(idImagette, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    allocation_tableau(ImgOut_imagette, OCTET, nTaille_imagette*3);
    resize_imagetteCouleur(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
    
    for(int j = 0; j < nTaille_imagette; j++){
      moyenneImagette += ImgIn_imagette[j];
    }
    moyenneImagette /= float(nTaille_imagette);
    moyennesImagettes[idImagette] = moyenneImagette;

    free(ImgIn_imagette);
  }

  for (int i=0; i < nTaille3; i+=3)
  {
    nR = ImgIn[i];
    nG = ImgIn[i+1];
    nB = ImgIn[i+2];
    

  }
  ecrire_image_ppm(cNomImgEcrite, ImgOut,  nH, nW);
  free(ImgIn); free(ImgOut);

  return 1;
}