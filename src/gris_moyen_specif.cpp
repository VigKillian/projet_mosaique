#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#include <float.h>
#include <fstream>
#define NB_BASE_DE_DONNEE 10000
using namespace std;
#include <atomic>
std::atomic<float> progress = 0.0f;



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
  std::ofstream resetProgress0("progress0.txt");
  if (resetProgress0.is_open()) {
      resetProgress0 << 0.0f;
      resetProgress0.close();
  }
  std::ofstream resetProgress1("progress1.txt");
  if (resetProgress1.is_open()) {
      resetProgress1 << 0.0f;
      resetProgress1.close();
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

  int cptImagette = 0;
  // Charger les imagettes et calculer leur moyenne de luminosité
  for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++){
    cptImagette++;
    float currentProgress = static_cast<float>(cptImagette) / NB_BASE_DE_DONNEE;      
    std::ofstream progressFile0("progress0.txt");
    if (progressFile0.is_open()) {
        progressFile0 << currentProgress;
        progressFile0.close();
    }
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

  int cptBloc=0;
  int nbBlocTotal = nTaille/(tailleBloc*tailleBloc);
  // Remplacement des blocs de l'image originale
  for(int i = 0; i <= nH - tailleBloc; i += tailleBloc){
    for(int j = 0; j <= nW - tailleBloc; j += tailleBloc){
      cptBloc++;
      float currentProgress = static_cast<float>(cptBloc) / nbBlocTotal;
      
      std::ofstream progressFile1("progress1.txt");
      if (progressFile1.is_open()) {
          progressFile1 << currentProgress;
          progressFile1.close();
      }
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
          if(differenceMin == 0) break;
        }
      }

      // Charger l'imagette sélectionnée et la réduire si nécessaire
      OCTET *ImgOut_imagette;
      OCTET *ImgIn_imagette;
      OCTET* blocSpecifie;
      OCTET* blocIn;
      int nH_imagette, nW_imagette, nTaille_imagette;
      
      loadImagette(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
      allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc);
      resize_imagette(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
      allocation_tableau(blocIn, OCTET, tailleBloc*tailleBloc);
      allocation_tableau(blocSpecifie, OCTET, tailleBloc*tailleBloc);

      for(int k = 0; k < tailleBloc; k++){
        for(int p = 0; p < tailleBloc; p++){
          blocIn[k * tailleBloc + p] = ImgIn[pixelDepart + k * nW + p];
        }
      }

      

      specification(ImgOut_imagette, blocIn, blocSpecifie, tailleBloc, tailleBloc);
      
      // Remplacer les pixels du bloc par ceux de l'imagette
      for(int k = 0; k < tailleBloc; k++){
        for(int p = 0; p < tailleBloc; p++){
          ImgOut[pixelDepart + k * nW + p] = blocSpecifie[k * tailleBloc + p];
        }
      }
      
      free(blocIn);
      free(blocSpecifie);
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
