#ifndef POPULATION_H_
#define POPULATION_H_

#include "nn.h"

class Population
{
public:
   struct Individual
   {
      typedef std::vector<double> bias_type;

      int pieces_played;
      int pieces_won;
      int games_played;
      int games_won;
      int fitness;
      int id;

      bias_type  sa_param;
      NeuralNetwork nn;

      Individual();
      bool operator< ( const Individual& rhs );
   };

   std::vector<Individual>    _population;   // evolution
   
private:
   std::string                _nn_layers;    // NN layers setup
   int                        _next_id;      // ID to be assigned for the next individual
   int                        _size;         // size of population
   int                        _generation;   // generation #
   
   void Clone( int n );
   void DisplayTop( int n );
   
public:
   bool Restart( const char* filename );
   bool Load( const char* filename );
   bool Save( const char* filename );
   
   void EvolveNN( int gen );
   void EvolveRM( int gen );
   void EvolveAB( int gen );

   void PlayARM( int num );
   void PlayAAB( int num );

   int GetSize() const;
   int GetGeneration() const;
};

#endif
