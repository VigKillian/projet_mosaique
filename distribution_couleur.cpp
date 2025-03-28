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

using namespace std;

int main(int argc, char *argv[]) {
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW,nW3, nTaille ,nR,nG,nB,repetition;
  vector<ImagetteCouleur> listeImagettes;
  int tailleBloc;


  if (argc != 5) {
    printf("Usage: ImageIn.ppm mageOut.ppm taille_bloc avec_repetition?[1/0]\n");
    exit(1);
  }

  sscanf(argv[1], "%s", cNomImgLue);
  sscanf(argv[2], "%s", cNomImgEcrite);
  sscanf (argv[3],"%d",&tailleBloc);
  sscanf (argv[4],"%d",&repetition);


  std::chrono::time_point<std::chrono::high_resolution_clock> _t0 = std::chrono::high_resolution_clock::now();


  OCTET *ImgIn,*ImgOut;

  lire_nb_lignes_colonnes_image_ppm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;
  nW3 = 3*nW;
  int nTaille3 = nTaille * 3;
  allocation_tableau(ImgIn, OCTET, nTaille3);
  lire_image_ppm(cNomImgLue, ImgIn, nH * nW);
  allocation_tableau(ImgOut, OCTET, nTaille3);

  // Charger les imagettes et calculer leur moyenne de luminosité
  for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++){
    OCTET *ImgIn_imagette, *ImgOut_imagette;
    int nH_imagette, nW_imagette, nTaille_imagette;
    vector<vector<int>> distrImagette = vector<vector<int>>(tailleBloc*tailleBloc,vector<int>(3,0));
    loadImagette_cou(idImagette, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    allocation_tableau(ImgOut_imagette, OCTET, nTaille_imagette*3);
    resize_imagetteCouleur(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
    
    for(int j = 0; j < tailleBloc*tailleBloc*3; j+=3){
      for(int canal = 0;canal<3;canal++){
        distrImagette[j/3][canal] = ImgOut_imagette[j+canal];
      }
    }
    listeImagettes.push_back({idImagette,{0.f,0.f,0.f},vector<vector<int>>(256, vector<int>(3, 0)),distrImagette,0});
    free(ImgIn_imagette);
  }

  for (int i=0; i <= nH - tailleBloc; i+= tailleBloc)
  {
    for(int j=0;j<= nW3 - 3*tailleBloc;j+=3*tailleBloc){
      int pixelDepart = i * nW3 + j;
      vector<vector<int>> blocDistribution(tailleBloc * tailleBloc, vector<int>(3,0));
      for(int k = 0;k<tailleBloc;k++){
        for(int p=0;p<tailleBloc*3;p+=3){
          for(int canal = 0;canal<3;canal++){
            blocDistribution[(k * tailleBloc) + (p/3)][canal] = ImgIn[pixelDepart+k*nW3+p+canal];
          }
        }
      }

      float distanceMin = FLT_MAX;
      int best_imagette_id = -1;
      if((bool)repetition){
        for(ImagetteCouleur &imagette : listeImagettes){
          float distanceTotale = 0.f;
          for(int pixel = 0; pixel < tailleBloc * tailleBloc; pixel++) {
            float distance = 0.f;
            for(int canal=0;canal<3;canal++){
              distance += pow(imagette.distribution[pixel][canal] - blocDistribution[pixel][canal], 2);
            }
            distance = (float)sqrt(distance);
            distanceTotale += distance;
          }
          if (distanceTotale < distanceMin) {
            distanceMin = distanceTotale;
            best_imagette_id = imagette.ID;
          }
          
        }
      }else{
        for(ImagetteCouleur &imagette : listeImagettes){
          float distanceTotale = 0.f;
          for(int pixel = 0; pixel < tailleBloc * tailleBloc; pixel++) {
            float distance = 0.f;
            for(int canal=0;canal<3;canal++){
              distance += pow(imagette.distribution[pixel][canal] - blocDistribution[pixel][canal], 2);
            }
            distance = (float)sqrt(distance);
            distanceTotale += distance;
          }
          if (distanceTotale < distanceMin && !imagette.isUsed) {
            distanceMin = distanceTotale;
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

  std::chrono::time_point<std::chrono::high_resolution_clock> _t1 = std::chrono::high_resolution_clock::now();
  std::cout << "Photo-mosaïque produite en : "<<std::chrono::duration<double>(_t1-_t0).count() << "s" << std::endl;

  std::cout<<"PSNR par rapport à l'image d'entrée : "<<calculer_PSNR(ImgIn, ImgOut, nTaille)<<"dB"<<std::endl;

  ecrire_image_ppm(cNomImgEcrite, ImgOut,  nH, nW);
  free(ImgIn); free(ImgOut);
  
  cout<<endl;

  return 1;
}