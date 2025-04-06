// test_couleur.cpp : Seuille une image en niveau de gris
#define NB_BASE_DE_DONNEE 9765
#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#include <float.h>
#include <fstream>

using namespace std;
#include <atomic>
std::atomic<float> progress = 0.0f;

int main(int argc, char *argv[]) {
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW,nW3, nTaille ,repetition;
  vector<ImagetteCouleur> listeImagettes;
  int tailleBloc;


  if (argc != 5) {
    printf("Usage: ImageIn.ppm ImageOut.ppm taille_bloc avec_repetition?[1/0]\n");
    exit(1);
  }
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
  sscanf(argv[1], "%s", cNomImgLue);
  sscanf(argv[2], "%s", cNomImgEcrite);
  sscanf (argv[3],"%d",&tailleBloc);
  sscanf (argv[4],"%d",&repetition);





  OCTET *ImgIn,*ImgOut;

  lire_nb_lignes_colonnes_image_ppm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;
  nW3 = 3*nW;
  int nTaille3 = nTaille * 3;
  allocation_tableau(ImgIn, OCTET, nTaille3);
  lire_image_ppm(cNomImgLue, ImgIn, nH * nW);
  allocation_tableau(ImgOut, OCTET, nTaille3);
  

  std::chrono::time_point<std::chrono::high_resolution_clock> _t0 = std::chrono::high_resolution_clock::now();

  OCTET* ImgNDG;
  allocation_tableau(ImgNDG, OCTET, nTaille);
  ndg(ImgIn, ImgNDG, nH, nW);

  OCTET* ImgSeuil;
  allocation_tableau(ImgSeuil, OCTET, nTaille);
  seuil_otsu(ImgNDG, ImgSeuil, nH, nW);
  free(ImgNDG);

  dilatation(ImgSeuil, nH, nW, 10);
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
    OCTET *ImgIn_imagette, *ImgOut_imagette;
    int nH_imagette, nW_imagette, nTaille_imagette;
    vector<float> moyen =  {0.f,0.f,0.f};
    loadImagette_cou(idImagette, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    allocation_tableau(ImgOut_imagette, OCTET, nTaille_imagette*3);
    resize_imagetteCouleur(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
    
    for(int j = 0; j < tailleBloc*tailleBloc*3; j+=3){
      for(int canal = 0;canal<3;canal++){
        moyen[canal] += ImgOut_imagette[j+canal];
      }
    }
    for(int canal = 0;canal<3;canal++){
      moyen[canal] /= (float) (tailleBloc*tailleBloc);
    }
    listeImagettes.push_back({idImagette,moyen,vector<vector<int>>(256, vector<int>(3, 0)),vector<vector<int>>(256, vector<int>(3, 0)),0});
    free(ImgIn_imagette);
  }

  // Première passe : zones importantes
  int cptBloc=0;
  int nbBlocTotal = nTaille/(tailleBloc*tailleBloc);
  for (int i=0; i <= nH - tailleBloc; i+= tailleBloc)
  {
    for(int j=0;j<= nW3 - 3*tailleBloc;j+=3*tailleBloc){
      cptBloc++;
      float currentProgress = static_cast<float>(cptBloc) / nbBlocTotal;
      
      std::ofstream progressFile1("progress1.txt");
      if (progressFile1.is_open()) {
          progressFile1 << currentProgress;
          progressFile1.close();
      }
      bool zoneImportante = false;
      int nbZoneImp = 0;
      // Check si le bloc de l'image seuillée contient une zone importante (au moins le quart des pixels sont noirs)
      for(int k = 0; k<tailleBloc; k++){
        for(int p = 0; p<tailleBloc; p++){
          if(ImgSeuil[i*nW+(j/3) + k*nW+p] == 0) nbZoneImp++;
        }
      }
      if(nbZoneImp >= 1) zoneImportante = true;
      if(zoneImportante){
        int pixelDepart = i * nW3 + j;
        vector<float> moyen_bloc  = vector<float>(3,0.f);
        for(int k = 0;k<tailleBloc;k++){
          for(int p=0;p<tailleBloc*3;p+=3){
            for(int canal = 0;canal<3;canal++){
              moyen_bloc[canal] +=ImgIn[pixelDepart+k*nW3+p+canal];
            }
          }
        }
        for(int canal = 0;canal<3;canal++){
          moyen_bloc[canal] /=(float) (tailleBloc*tailleBloc);
        }
        float distanceMin = FLT_MAX;
        int best_imagette_id = -1;
        if((bool)repetition){
            for(ImagetteCouleur &imagette : listeImagettes){
            float distance = 0.f;
            for(int canal=0;canal<3;canal++){
              distance += pow(imagette.moyen[canal]-moyen_bloc[canal],2);
            }
            distance = sqrt(distance);
            if(distance<distanceMin ){
              distanceMin = distance;
              best_imagette_id = imagette.ID;
            }
          }
        }else{
          for(ImagetteCouleur &imagette : listeImagettes){
            float distance = 0.f;
            for(int canal=0;canal<3;canal++){
              distance += pow(imagette.moyen[canal]-moyen_bloc[canal],2);
            }
            distance = sqrt(distance);
            if(distance<distanceMin && !imagette.isUsed){
              distanceMin = distance;
              best_imagette_id = imagette.ID;
            }
          }
          // Mise à jour de l'état utilisé
          for(ImagetteCouleur &imagette : listeImagettes){
            if(imagette.ID == best_imagette_id){
              imagette.isUsed = 1;
              break; 
            }
          }
        }

        // Charger l'imagette sélectionnée et la réduire si nécessaire
        OCTET *ImgOut_imagette;
        OCTET *ImgIn_imagette;
        int nH_imagette, nW_imagette, nTaille_imagette;
        
        loadImagette_cou(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
        allocation_tableau(ImgOut_imagette, OCTET, nTaille_imagette*3);
        resize_imagetteCouleur(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);

        // Remplacer les pixels du bloc par ceux de l'imagette
        for(int k = 0; k < tailleBloc; k++){
          for(int p = 0; p < tailleBloc*3; p+=3){
            for (int canal = 0;canal<3;canal++){
              ImgOut[pixelDepart + k * nW3 + p+canal] = ImgOut_imagette[k * tailleBloc*3 + p+canal];
            }
          }
        }
        
        free(ImgIn_imagette);
        free(ImgOut_imagette);
      }
    }
  }

  // Seconde passe : zones moins importantes
  cptBloc=0;
  nbBlocTotal = nTaille/(tailleBloc*tailleBloc);
  for (int i=0; i <= nH - tailleBloc; i+= tailleBloc)
  {
    for(int j=0;j<= nW3 - 3*tailleBloc;j+=3*tailleBloc){
      cptBloc++;
      float currentProgress = static_cast<float>(cptBloc) / nbBlocTotal;
      
      std::ofstream progressFile1("progress1.txt");
      if (progressFile1.is_open()) {
          progressFile1 << currentProgress;
          progressFile1.close();
      }
      bool zoneImportante = false;
      int nbZoneImp = 0;
      // Check si le bloc de l'image seuillée contient une zone importante (au moins le quart des pixels sont noirs)
      for(int k = 0; k<tailleBloc; k++){
        for(int p = 0; p<tailleBloc; p++){
          if(ImgSeuil[i*nW+(j/3) + k*nW+p] == 0) nbZoneImp++;
        }
      }
      if(nbZoneImp >= 1) zoneImportante = true;
      if(!zoneImportante){
        int pixelDepart = i * nW3 + j;
        vector<float> moyen_bloc  = vector<float>(3,0.f);
        for(int k = 0;k<tailleBloc;k++){
          for(int p=0;p<tailleBloc*3;p+=3){
            for(int canal = 0;canal<3;canal++){
              moyen_bloc[canal] +=ImgIn[pixelDepart+k*nW3+p+canal];
            }
          }
        }
        for(int canal = 0;canal<3;canal++){
          moyen_bloc[canal] /=(float) (tailleBloc*tailleBloc);
        }
        float distanceMin = FLT_MAX;
        int best_imagette_id = -1;
        if((bool)repetition){
            for(ImagetteCouleur &imagette : listeImagettes){
            float distance = 0.f;
            for(int canal=0;canal<3;canal++){
              distance += pow(imagette.moyen[canal]-moyen_bloc[canal],2);
            }
            distance = sqrt(distance);
            if(distance<distanceMin ){
              distanceMin = distance;
              best_imagette_id = imagette.ID;
            }
          }
        }else{
          for(ImagetteCouleur &imagette : listeImagettes){
            float distance = 0.f;
            for(int canal=0;canal<3;canal++){
              distance += pow(imagette.moyen[canal]-moyen_bloc[canal],2);
            }
            distance = sqrt(distance);
            if(distance<distanceMin && !imagette.isUsed){
              distanceMin = distance;
              best_imagette_id = imagette.ID;
            }
          }
          // Mise à jour de l'état utilisé
          for(ImagetteCouleur &imagette : listeImagettes){
            if(imagette.ID == best_imagette_id){
              imagette.isUsed = 1;
              break; 
            }
          }
        }

        // Charger l'imagette sélectionnée et la réduire si nécessaire
        OCTET *ImgOut_imagette;
        OCTET *ImgIn_imagette;
        int nH_imagette, nW_imagette, nTaille_imagette;
        
        loadImagette_cou(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
        allocation_tableau(ImgOut_imagette, OCTET, nTaille_imagette*3);
        resize_imagetteCouleur(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);

        // Remplacer les pixels du bloc par ceux de l'imagette
        for(int k = 0; k < tailleBloc; k++){
          for(int p = 0; p < tailleBloc*3; p+=3){
            for (int canal = 0;canal<3;canal++){
              ImgOut[pixelDepart + k * nW3 + p+canal] = ImgOut_imagette[k * tailleBloc*3 + p+canal];
            }
          }
        }
        
        free(ImgIn_imagette);
        free(ImgOut_imagette);
      }
    }
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> _t1 = std::chrono::high_resolution_clock::now();
  std::cout << "Photo-mosaïque produite en : "<<std::chrono::duration<double>(_t1-_t0).count() << "s" << std::endl;

  std::cout<<"PSNR par rapport à l'image d'entrée : "<<calculer_PSNR(ImgIn, ImgOut, nTaille)<<"dB"<<std::endl;

  ecrire_image_ppm(cNomImgEcrite, ImgOut,  nH, nW);
  free(ImgIn); free(ImgOut);

  return 1;
}