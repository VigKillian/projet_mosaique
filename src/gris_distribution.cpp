#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <cmath>
#include <float.h>
#include <fstream>
#define NB_BASE_DE_DONNEE 10000
using namespace std;
#include <atomic>
std::atomic<float> progress = 0.0f;
/*
L'idée est que, au lieu de comparer d'abord l'histogramme pour sélectionner l'imagette la plus adaptée, 
puis de la réduire, on réduit d'abord l'imagette avant de faire la comparaison.
*/

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, tailleBloc,repetition;
  vector<ImagetteDis> listeImagettes;

  if (argc != 5) 
    {
      printf("Usage: ImageIn.pgm ImageOut.pgm taille_bloc avec_repetition?[1/0]\n"); 
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
  sscanf (argv[1],"%s", cNomImgLue);
  sscanf (argv[2],"%s", cNomImgEcrite);
  sscanf (argv[3],"%d", &tailleBloc);
  sscanf (argv[4],"%d",&repetition);


  OCTET *ImgIn, *ImgOut;

  lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;

  allocation_tableau(ImgIn, OCTET, nTaille);
  lire_image_pgm(cNomImgLue, ImgIn, nTaille);
  allocation_tableau(ImgOut, OCTET, nTaille);
  int cptImagette = 0;

  // Charger et réduire toutes les imagettes de la base de données
  for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++) {
    cptImagette++;
    float currentProgress = static_cast<float>(cptImagette) / NB_BASE_DE_DONNEE;      
    std::ofstream progressFile0("progress0.txt");
    if (progressFile0.is_open()) {
        progressFile0 << currentProgress;
        progressFile0.close();
    }
    OCTET *ImgIn_imagette, *ImgOut_imagette;
    int nH_imagette, nW_imagette, nTaille_imagette;
    
    loadImagette(idImagette, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc);
    resize_imagette(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
    
    vector<int> distributionImagette(tailleBloc * tailleBloc, 0);
    for(int j = 0; j < tailleBloc * tailleBloc; j++) {
      distributionImagette[j] = ImgOut_imagette[j];
    }

    listeImagettes.push_back({idImagette, 0.f, distributionImagette, 0});
    free(ImgIn_imagette);free(ImgOut_imagette);
  }
  int cptBloc=0;
  int nbBlocTotal = nTaille/(tailleBloc*tailleBloc);
  // Remplacement des blocs de l'image originale
  for(int i = 0; i <= nH - tailleBloc; i += tailleBloc) {
    for(int j = 0; j <= nW - tailleBloc; j += tailleBloc) {
      cptBloc++;
      float currentProgress = static_cast<float>(cptBloc) / nbBlocTotal;
      
      std::ofstream progressFile1("progress1.txt");
      if (progressFile1.is_open()) {
          progressFile1 << currentProgress;
          progressFile1.close();
      }
      int pixelDepart = i * nW + j;
      vector<int> blocDistribution(tailleBloc * tailleBloc, 0);
      
      for(int k = 0; k < tailleBloc; k++) {
        for(int p = 0; p < tailleBloc; p++) {
          blocDistribution[k * tailleBloc + p] = ImgIn[pixelDepart + k * nW + p];
        }
      }

      int best_imagette_id = -1;
      float distanceMin = FLT_MAX;
      if((bool)repetition){
        for (ImagetteDis &imagette : listeImagettes) {
          float distanceTotale = 0.f;
          for(int pixel = 0; pixel < tailleBloc * tailleBloc; pixel++) {
            distanceTotale += (float)sqrt(pow(imagette.distribution[pixel] - blocDistribution[pixel], 2));
          }
          if (distanceTotale < distanceMin) {
            distanceMin = distanceTotale;
            best_imagette_id = imagette.ID;
          }
        }
      }else{
        for (ImagetteDis &imagette : listeImagettes) {
          float distanceTotale = 0.f;
          for(int pixel = 0; pixel < tailleBloc * tailleBloc; pixel++) {
            distanceTotale += (float)sqrt(pow(imagette.distribution[pixel] - blocDistribution[pixel], 2));
          }
          if (distanceTotale < distanceMin && !imagette.isUsed) {
            distanceMin = distanceTotale;
            best_imagette_id = imagette.ID;
          }
        }
        // Mise à jour de l'état utilisé
        for(ImagetteDis &imagette : listeImagettes){
          if(imagette.ID == best_imagette_id){
            imagette.isUsed = 1;
            break; 
          }
        }
      }


      // Charger l'imagette sélectionnée et la réduire si nécessaire
      OCTET *ImgIn_imagette, *ImgOut_imagette;
      int nH_imagette, nW_imagette, nTaille_imagette;
      loadImagette(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
      allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc);
      resize_imagette(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
      
      // Remplacer les pixels du bloc par ceux de l'imagette
      for(int k = 0; k < tailleBloc; k++) {
        for(int p = 0; p < tailleBloc; p++) {
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
