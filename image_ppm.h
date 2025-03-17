// ----------------------------------------------------------------------------
// Filename        : image_ppm.c
// Description     :
// Created On      : Tue Mar 31 13:26:36 2005
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <iostream>
#include <array>
#include <chrono>
#include <vector>
using namespace std;

#define allocation_tableau(nom, type, nombre) \
if( (nom = (type*) calloc (nombre, sizeof(type) ) ) == NULL ) \
{\
 printf("\n Allocation dynamique impossible pour un pointeur-tableau \n");\
 exit(1);\
}

typedef unsigned char OCTET;

/*===========================================================================*/
void ignorer_commentaires(FILE * f)
{
  unsigned char c;
  while((c=fgetc(f)) == '#')
    while((c=fgetc(f)) != '\n');
  fseek(f, -sizeof(unsigned char), SEEK_CUR);
}
/*===========================================================================*/


/*===========================================================================*/
void ecrire_image_ppm(char  nom_image[], OCTET *pt_image, int nb_lignes, int nb_colonnes)
{
   FILE *f_image;
   int taille_image = 3*nb_colonnes * nb_lignes;

   if( (f_image = fopen(nom_image, "wb")) == NULL)
      {
	 printf("\nPas d'acces en ecriture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fprintf(f_image,"P6\r") ;                               /*ecriture entete*/
	 fprintf(f_image,"%d %d\r255\r", nb_colonnes, nb_lignes) ;

	 if( (fwrite((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     != (size_t)(taille_image))
	    {
	       printf("\nErreur d'ecriture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}
/*===========================================================================*/

/*===========================================================================*/		
void lire_nb_lignes_colonnes_image_ppm(char nom_image[], int *nb_lignes, int *nb_colonnes)
{
   FILE *f_image;
   int max_grey_val;

   /* cf : l'entete d'une image .ppm : P6                   */
   /*				       nb_colonnes nb_lignes */
   /*    			       max_grey_val          */


   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fscanf(f_image, "P6 ");
	 ignorer_commentaires(f_image);
	 fscanf(f_image, "%d %d %d%*c", nb_colonnes, nb_lignes, &max_grey_val);
	 fclose(f_image);
      }
}
/*===========================================================================*/
/*===========================================================================*/
void lire_image_ppm(char  nom_image[], OCTET *pt_image, int taille_image)
{
   FILE *f_image;
   int  nb_colonnes, nb_lignes, max_grey_val;
   taille_image=3*taille_image;

   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	fscanf(f_image, "P6 ");
	ignorer_commentaires(f_image);
	fscanf(f_image, "%d %d %d%*c",
	       &nb_colonnes, &nb_lignes, &max_grey_val); /*lecture entete*/

	 if( (fread((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     !=  (size_t)(taille_image))
	    {
	       printf("\nErreur de lecture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}

/*===========================================================================*/
/*===========================================================================*/

void planR(OCTET *pt_image, OCTET *src, int taille_image){
   int i;
   for (i=0; i<taille_image; i++){
      pt_image[i]=src[3*i];
      }
   }
   
/*===========================================================================*/
/*===========================================================================*/

void planV(OCTET *pt_image, OCTET *src, int taille_image){
   int i;
   for (i=0; i<taille_image; i++){
      pt_image[i]=src[3*i+1];
      }
   }   

/*===========================================================================*/
/*===========================================================================*/

void planB(OCTET *pt_image, OCTET *src, int taille_image){
   int i;
   for (i=0; i<taille_image; i++){
      pt_image[i]=src[3*i+2];
      }
   }
   
/*===========================================================================*/   
/*===========================================================================*/

void ecrire_image_pgm(char  nom_image[], OCTET *pt_image, int nb_lignes, int nb_colonnes)
{
   FILE *f_image;
   int taille_image = nb_colonnes * nb_lignes;

   if( (f_image = fopen(nom_image, "wb")) == NULL)
      {
	 printf("\nPas d'acces en ecriture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fprintf(f_image,"P5\r") ;                               /*ecriture entete*/
	 fprintf(f_image,"%d %d\r255\r", nb_colonnes, nb_lignes) ;

	 if( (fwrite((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     != (size_t) taille_image)
	    {
	       printf("\nErreur de lecture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}
/*===========================================================================*/

void lire_nb_lignes_colonnes_image_pgm(char nom_image[], int *nb_lignes, int *nb_colonnes)
{
   FILE *f_image;
   int max_grey_val;

   /* cf : l'entete d'une image .pgm : P5                    */
   /*				       nb_colonnes nb_lignes */
   /*    			       max_grey_val          */


   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fscanf(f_image, "P5 ");
	 ignorer_commentaires(f_image);
	 fscanf(f_image, "%d %d %d%*c", nb_colonnes, nb_lignes, &max_grey_val);
	 fclose(f_image);
      }
}
/*===========================================================================*/
/*===========================================================================*/
void lire_image_pgm(char  nom_image[], OCTET *pt_image, int taille_image)
{
   FILE *f_image;
   int  nb_colonnes, nb_lignes, max_grey_val;

   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	fscanf(f_image, "P5 ");
	ignorer_commentaires(f_image);
	fscanf(f_image, "%d %d %d%*c",
	       &nb_colonnes, &nb_lignes, &max_grey_val); /*lecture entete*/

	 if( (fread((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     !=  (size_t) taille_image)
	    {
	       printf("\nErreur de lecture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}
/*===========================================================================*/
/*===========================================================================*/
void resize_imagette(const unsigned char* img_in, int h_in, int w_in,unsigned char* img_out, int h_out, int w_out) 
{
  int block_h = h_in / h_out;  // Facteur de réduction en hauteur
  int block_w = w_in / w_out;  // Facteur de réduction en largeur

  for (int y = 0; y < h_out; y++) {
    for (int x = 0; x < w_out; x++) {
      int sum = 0;
      int count = 0;

      // Parcourir le bloc correspondant dans l'image d'origine
      for (int i = 0; i < block_h; i++) {
        for (int j = 0; j < block_w; j++) {
          int orig_y = y * block_h + i;
          int orig_x = x * block_w + j;
          sum += img_in[orig_y * w_in + orig_x];
          count++;
        }
      }
      
      // Mettre la moyenne dans l'image de sortie
      img_out[y * w_out + x] = sum / count;
    }
  }
}
/*===========================================================================*/
/*===========================================================================*/
void resize_imagetteCouleur(const unsigned char* img_in, int h_in, int w_in, unsigned char* img_out, int h_out, int w_out) 
{
  int block_h = h_in / h_out;  // Facteur de réduction en hauteur
  int block_w = w_in / w_out;  // Facteur de réduction en largeur

  for (int y = 0; y < h_out; y++) {
    for (int x = 0; x < w_out; x++) {
      int sum_r = 0, sum_g = 0, sum_b = 0;
      int count = 0;

      for (int i = 0; i < block_h; i++) {
        for (int j = 0; j < block_w; j++) {
          int orig_y = y * block_h + i;
          int orig_x = (x * block_w + j) * 3;  

          sum_r += img_in[orig_y * w_in * 3 + orig_x];      
          sum_g += img_in[orig_y * w_in * 3 + orig_x + 1];  
          sum_b += img_in[orig_y * w_in * 3 + orig_x + 2]; 
          count++;
        }
      }

      int dest_index = (y * w_out + x) * 3;   
      img_out[dest_index]     = sum_r / count;   
      img_out[dest_index + 1] = sum_g / count;  
      img_out[dest_index + 2] = sum_b / count; 
    }
  }
}

/*===========================================================================*/
/*===========================================================================*/
void loadImagette(int id_imagette,OCTET*& ImgIn_tile,int& nH_tile,int& nW_tile,int& nTaille_tile){
  string nom_file = "./img_tile/"+std::to_string(id_imagette)+".pgm";
  lire_nb_lignes_colonnes_image_pgm((char*)nom_file.c_str(),&nH_tile,&nW_tile);
  nTaille_tile = nW_tile*nH_tile;
  allocation_tableau(ImgIn_tile, OCTET, nTaille_tile);
  lire_image_pgm((char*)nom_file.c_str(), ImgIn_tile, nTaille_tile);
}
/*===========================================================================*/
/*===========================================================================*/
void loadImagette_cou(int id_imagette,OCTET*& ImgIn_tile,int& nH_tile,int& nW_tile,int& nTaille_tile){
  string nom_file = "./img_tile_color/i"+std::to_string(id_imagette)+".ppm";
  lire_nb_lignes_colonnes_image_ppm((char*)nom_file.c_str(),&nH_tile,&nW_tile);
  nTaille_tile = nW_tile*nH_tile;
  int nTaille_tile3 = nTaille_tile*3;
  allocation_tableau(ImgIn_tile, OCTET, nTaille_tile3);
  lire_image_ppm((char*)nom_file.c_str(), ImgIn_tile, nTaille_tile);
}
/*===========================================================================*/
/*===========================================================================*/
struct Imagette{
  int ID;
  float moyen;
  std::array<int, 256> histo;
  bool isUsed;

    // constructeur par defaut
  Imagette(int id=0, float moy = 0.f,std::array<int, 256> his = {0}, bool is_used = 0):
    ID(id),moyen(moy),histo(his),isUsed(is_used){}
};
/*===========================================================================*/
/*===========================================================================*/
struct ImagetteCouleur{
  int ID;
  vector<float> moyen;
  vector<vector<int>> histo;
  vector<vector<int>> distribution;
  bool isUsed;

    // constructeur par defaut
  ImagetteCouleur(int id=0, vector<float> moy = vector<float>(3,0.f),vector<vector<int>> his =vector<vector<int>>(256, vector<int>(3, 0)), vector<vector<int>> dis = vector<vector<int>>(256,vector<int>(3,0)),bool is_used = 0):
    ID(id),moyen(moy),histo(his),distribution(dis),isUsed(is_used){}
};
/*===========================================================================*/
/*===========================================================================*/

struct ImagetteDis{
  int ID;
  float moyen;
  std::vector<int> distribution;
  bool isUsed;

    // constructeur par defaut
  ImagetteDis(int id=0, float moy = 0.f,std::vector<int> dis = std::vector<int>(256,0), bool is_used = 0):
    ID(id),moyen(moy),distribution(dis),isUsed(is_used){}
};
/*===========================================================================*/
/*===========================================================================*/

double calculer_PSNR(OCTET* ImgOriginale, OCTET* ImgReconstruite, int nTaille) {
    double mse = 0.0;
    for (int i = 0; i < nTaille; i++) {
        double diff = ImgOriginale[i] - ImgReconstruite[i];
        mse += diff * diff;
    }
    mse /= nTaille;
    if (mse == 0) return INFINITY;
    double psnr = 10 * log10((255.0 * 255.0) / mse);
    return psnr;
}
/*===========================================================================*/