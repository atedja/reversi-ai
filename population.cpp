#include "lib.h"
#include "population.h"
#include "reversi.h"
#include "handler.h"
#include "common.h"

const int FITNESS_WIN  =  1;
const int FITNESS_LOSE = -2;
const int FITNESS_DRAW =  0;
const double DEG2RAD = 0.0174532925;


inline int to_int( std::string s )
{
   std::stringstream ss(s);
   int x; ss >> x;
   return x;
}

Population::Individual::Individual() :
fitness(0), id(-1), pieces_won(0), pieces_played(0), games_won(0), games_played(0)
{
}

// Pay attention to the negative logic (< and >)
bool Population::Individual::operator< ( const Population::Individual& rhs )
{
   // sort by fitness level, then games won, then pieces won
   if ( fitness == rhs.fitness )
   {
      double tf = double(games_won)/games_played;
      double rf = double(rhs.games_won)/rhs.games_played;
      if ( tf == rf )
      {
         double tpf = double(pieces_won)/pieces_played;
         double rpf = double(rhs.pieces_won)/rhs.pieces_played;
         return tpf > rpf;
      }
      return tf > rf;
   }

   return fitness > rhs.fitness;
}


// DisplayTop()
// Displays the top 'n' neural networks.
void Population::DisplayTop( int n )
{
   using std::cout;
   
   cout << "This generation result:\n";
   cout << "#\tID\tfitness\twin\tgames\tp-won\tp-play\tp-percent\n";
   cout << "------------------------------------------------------------------\n";
   for( int i=0; i<n; ++i )
   {
      float percent = float(_population[i].pieces_won)/_population[i].pieces_played * 100.0f;
      cout << "#" << i << "\t" << _population[i].id << "\t" << _population[i].fitness << "\t" <<
         _population[i].games_won << "\t" << _population[i].games_played << "\t" << _population[i].pieces_won <<
         "\t" << _population[i].pieces_played << "\t" << percent << "\n";
   }
   cout.flush();
}

// Clone()
// Clones the first 'n' individuals with slight random adjustments, removing the others.
// Gaussian mutation.
void Population::Clone( int n )
{
   std::cout << "Cloning neural networks..." << std::endl;

   int wc = _population[0].nn.WeightCount();
   int cl = n; int num_clones = (_size/n)-1;
   double taup = 1.0/sqrt(2.0*sqrt(double(wc))); // tau'
   double tau = 1.0/sqrt(2.0*wc);
   for( int i=0; i<n; ++i )   // Clone the first n
   {
      double Ni = gaussrandf(0.0,1.0);
      for (int c=0; c<num_clones; ++c )   // Make 'num_clones' clones for each n
      {
         NeuralNetwork::weight_type wwc = _population[i].nn.GetWeights();
         Individual::bias_type tsa_param(wc);

         // adjust weight and bias based on the formula:
         // sigma'(j) = sigma(j) * exp(taup*Nw(0,1)+tau*Ni(0,1))
         // weight'(j) = weight(j) + sigma'(j)*Nw(0,1)
         for( int w=0; w<wc; ++w )
         {
            double Nw = gaussrandf(0.0,1.0);
            tsa_param[w] = _population[i].sa_param[w] * exp(taup*Nw+tau*Ni);
            wwc[w].weight = wwc[w].weight + tsa_param[w]*Nw;
         }

         _population[cl].nn.ReplaceWeight( wwc );
         _population[cl].sa_param = tsa_param;
         _population[cl].id = _next_id;
         ++_next_id;
         ++cl;
      }
   }

   // reset fitness level
   for( int i=0; i<_size; ++i )
   {
      _population[i].pieces_played = 0;
      _population[i].pieces_won = 0;
      _population[i].fitness = 0;
      _population[i].games_played = 0;
      _population[i].games_won = 0;
   }
}

// Restart()
// Restarts the evolution process using the configuration specified in the file 'filename'
// by creating a new population with 'num' individuals.
bool Population::Restart( const char* filename )
{
   // open the config file
   std::ifstream file( filename );
   if ( file.is_open() )
   {
      std::string line;
      std::getline( file, line );
      std::stringstream ss(line);
      ss >> _size;
      std::getline( file, _nn_layers );
   }
   else
   {
      std::cout << "Error in opening file: " << filename << std::endl;
      return false;
   }
   
   std::cout << "Restarting evolution...\nPopulation size: " << _size << "\n";
   std::cout << "NN Layer Configuration: " << _nn_layers << std::endl;
   std::cout.flush();
   _generation = 0;
   
   // create individuals
   _population.resize(_size);
   for( int i=0; i<_size; ++i )
   {
      std::cout << "\rCreating new neural networks..." << i; std::cout.flush();
      _population[i].nn.Create( _nn_layers.c_str() );
      _population[i].id = i;

      // initialize self-adaptive parameters
      int wc = _population[i].nn.WeightCount();
      _population[i].sa_param.resize(wc);
      for( int w=0; w<wc; ++w )
         _population[i].sa_param[w] = randf(-0.5,0.5);
   }
   _next_id = _size;
   std::cout << "\n";
   
   return true;
}

// Load()
// Loads population from file 'filename'. Loads all if 'all' is true, otherwise just Coconut
bool Population::Load( const char* filename )
{
   std::ifstream file( filename );
   if ( file.is_open() )
   {
      std::string line;
      std::getline( file, line );                     // Read population size
      _size = to_int(line);
      _population.resize( _size );
      std::getline( file, _nn_layers );               // Read NN settings
      std::getline( file, line );                     // Read generation
      _generation = to_int(line);
      int nc, wc;
      std::getline( file, line ); nc = to_int(line);  // Read node count
      std::getline( file, line ); wc = to_int(line);  // Read weight count
      std::getline( file, line );                     // Read next ID
      _next_id = to_int(line);

      NeuralNetwork::weight_type ccw(wc);
      Individual::bias_type      tsa_param(wc);
      int nni = 0;
      while( nni < _size )                            // Read weights for all NN
      {
         std::cout << "\rLoading neural networks..." << nni; std::cout.flush();
         getline( file, line );                       // Read weights for each NN
         std::stringstream ss(line);
         ss >> _population[nni].id;

         // load weights and biases
         for( int w=0; w<wc; ++w )
         {
            ss >> tsa_param[w];
            ss >> ccw[w].weight;
         }
         _population[nni].nn.Create( _nn_layers.c_str(), ccw );
         _population[nni].sa_param = tsa_param;
         nni++;
      }
      
      file.close();
      std::cout << "\n";
   }
   else
   {
      return Restart( "nn.conf" );
   }
   return true;
}

// Save()
// Saves population to file 'filename'.
// Format:
// [POPULATION SIZE]
// [NN LAYER SETUP]
// [GENERATION #]
// [#NODES IN EACH NN]
// [#WEIGHTS IN EACH NN]
// [ID FOR NEXT INDIVIDUAL]
// [ID_NN(0)] [WEIGHTS OF NN(0)]
// [ID_NN(1)] [WEIGHTS OF NN(1)]
// ...
// [ID_NN(n-1)] [WEIGHTS OF NN(n-1)]
bool Population::Save( const char* filename )
{
   std::ofstream file;
   file.open( filename );
   if ( file.is_open() )
   {
      file << _size << "\n";
      file << _nn_layers << "\n";
      file << _generation << "\n";
      file << _population[0].nn.NodesCount() << "\n";
      file << _population[0].nn.WeightCount() << "\n";
      file << _next_id << "\n";

      NeuralNetwork::weight_type ccw(_population[0].nn.WeightCount());
      Individual::bias_type tsa_param(_population[0].nn.WeightCount());
      for( int i=0; i<_size; ++i )
      {
         std::cout << "\rSaving neural networks..." << i; std::cout.flush();
         ccw = _population[i].nn.GetWeights();
         tsa_param = _population[i].sa_param;
         file << _population[i].id << " ";

         // save weights and biases
         for( unsigned int j=0; j<ccw.size(); ++j )
         {
            file << tsa_param[j] << " ";
            file << ccw[j].weight << " ";
         }
         file << "\n";
      }
      std::cout << "\n";

      file.close();
   }
   else
      return false;
      
   return true;
}


// EvolveNN()
// Evolves population for 'gen' generations against its own.
void Population::EvolveNN( int gen )
{
   Reversi game;
   int wpc, bpc;                             // wpc/bpc = white/black piece count
   NNComputer     computer_nn1( false );
   NNComputer     computer_nn2( false );
   Reversi::PlayerHandler* wp = 0;
   Reversi::PlayerHandler* bp = 0;
   int best_offset = _size/2;

   std::cout << "Starting evolution...\n";
   std::cout << "----------------------------------------------------------------------\n";
   for( int g=0; g<gen; ++g )
   {
   	std::cout << "GENERATION: " << _generation << "\n";
      for( int i=0; i<_size-1; ++i )
      {
         int pl = i;
         computer_nn1.SetNN( &_population[pl] );
         for ( int j=i+1; j<_size; ++j )
         {
            int opp = j;
            computer_nn2.SetNN( &_population[opp] );

            // NN1 WHITE, NN2 BLACK
            wpc = bpc = 0;
            computer_nn1.SetColor( Reversi::WHITE );
            computer_nn2.SetColor( Reversi::BLACK );
            wp = &computer_nn1;
            bp = &computer_nn2;
            game.Start( *wp, *bp );
            game.CountPieces( wpc, bpc );
            _population[pl].pieces_won += wpc;
            _population[pl].pieces_played += wpc+bpc;
            _population[pl].games_played++;
            _population[opp].pieces_won += bpc;
            _population[opp].pieces_played += wpc+bpc;
            _population[opp].games_played++;

            std::cout << "[" << _generation << "] " << _population[pl].id << "[W] vs. " << _population[opp].id << "[B]. ";
            if ( wpc > bpc )
            {
               // NN1 wins
               _population[pl].fitness += FITNESS_WIN;
               _population[pl].games_won++;
               _population[opp].fitness += FITNESS_LOSE;
               std::cout << "Result: " << _population[pl].id << " won. " << wpc << "-" << bpc << "\n";
            }
            else if ( wpc < bpc )
            {
               _population[pl].fitness += FITNESS_LOSE;
               _population[opp].fitness += FITNESS_WIN;
               _population[opp].games_won++;
               std::cout << "Result: " << _population[opp].id << " won. " << wpc << "-" << bpc << "\n";
            }
            else
            {
               std::cout << "Result: DRAW. " << wpc << "-" << bpc << "\n";
            }
            

            // swap sides
            // NN1 BLACK, NN2 WHITE
            wpc = bpc = 0;
            computer_nn1.SetColor( Reversi::BLACK );
            computer_nn2.SetColor( Reversi::WHITE );
            wp = &computer_nn2;
            bp = &computer_nn1;
            game.Start( *wp, *bp );
            game.CountPieces( wpc, bpc );
            _population[pl].pieces_won += bpc;
            _population[pl].pieces_played += wpc+bpc;
            _population[pl].games_played++;
            _population[opp].pieces_won += wpc;
            _population[opp].pieces_played += wpc+bpc;
            _population[opp].games_played++;
            
            std::cout << "[" << _generation << "] " << _population[pl].id << "[B] vs. " << _population[opp].id << "[W]. ";
            if ( wpc > bpc )
            {
               _population[pl].fitness += FITNESS_LOSE;
               _population[opp].fitness += FITNESS_WIN;
               _population[opp].games_won++;
               std::cout << "Result: " << _population[opp].id << " won. " << wpc << "-" << bpc << "\n";
            }
            else if ( wpc < bpc )
            {
               _population[pl].fitness += FITNESS_WIN;
               _population[pl].games_won++;
               _population[opp].fitness += FITNESS_LOSE;
               std::cout << "Result: " << _population[pl].id << " won. " << wpc << "-" << bpc << "\n";
            }
            else
            {
               std::cout << "Result: DRAW. " << wpc << "-" << bpc << "\n";;
            }
         }
      }
      
      // sort individuals based on fitness level
      std::sort( _population.begin(), _population.end() );
      
      DisplayTop( best_offset );
      
      // clone the top 50%
      Clone( best_offset );
      
      _generation++;
   }

   std::cout << "Measuring performance against a random mover 100 games:\n";
   PlayARM( 100 );

   std::cout << "Evolution Complete.\n";
}


// EvolveRM()
// Evolves population for 'gen' generations against the random mover.
void Population::EvolveRM( int gen )
{
   Reversi game;
   int wpc, bpc;                             // wpc/bpc = white/black piece count
   NNComputer     computer_nn( false );
   RandomComputer random_mover( false );
   Reversi::PlayerHandler* wp = 0;
   Reversi::PlayerHandler* bp = 0;
   int best_offset = _size/2;

   std::cout << "Starting evolution...\n";
   std::cout << "----------------------------------------------------------------------\n";
   for( int g=0; g<gen; ++g )
   {
   	std::cout << "GENERATION: " << _generation << "\n";
      for( int i=0; i<_size; ++i )
      {
         int pl = i;
         computer_nn.SetNN( &_population[pl] );
         for ( int j=0; j<10; ++j )
         {
            int opp = j;

            // NN1 WHITE, Random Mover BLACK
            wpc = bpc = 0;
            computer_nn.SetColor( Reversi::WHITE );
            random_mover.SetColor( Reversi::BLACK );
            wp = &computer_nn;
            bp = &random_mover;
            game.Start( *wp, *bp );
            game.CountPieces( wpc, bpc );
            _population[pl].pieces_won += wpc;
            _population[pl].pieces_played += wpc+bpc;
            _population[pl].games_played++;

            std::cout << "[" << _generation << "] " << _population[pl].id << "[W] vs. Random Mover[B]. ";
            if ( wpc > bpc )
            {
               // pl wins
               _population[pl].fitness += FITNESS_WIN;
               _population[pl].games_won++;
               std::cout << "Result: " << _population[pl].id << " won. " << wpc << "-" << bpc << "\n";
            }
            else if ( wpc < bpc )
            {
               _population[pl].fitness += FITNESS_LOSE;
               std::cout << "Result: Random Mover won. " << wpc << "-" << bpc << "\n";
            }
            else
            {
               std::cout << "Result: DRAW. " << wpc << "-" << bpc << "\n";
            }
            

            // swap sides
            // NN1 BLACK, Random Mover WHITE
            wpc = bpc = 0;
            computer_nn.SetColor( Reversi::BLACK );
            random_mover.SetColor( Reversi::WHITE );
            wp = &random_mover;
            bp = &computer_nn;
            game.Start( *wp, *bp );
            game.CountPieces( wpc, bpc );
            _population[pl].pieces_won += bpc;
            _population[pl].pieces_played += wpc+bpc;
            _population[pl].games_played++;
            
            std::cout << "[" << _generation << "] " << _population[pl].id << "[B] vs. Random Mover[W]. ";
            if ( wpc > bpc )
            {
               _population[pl].fitness += FITNESS_LOSE;
               std::cout << "Result: Random Mover won. " << wpc << "-" << bpc << "\n";
            }
            else if ( wpc < bpc )
            {
               _population[pl].fitness += FITNESS_WIN;
               _population[pl].games_won++;
               std::cout << "Result: " << _population[pl].id << " won. " << wpc << "-" << bpc << "\n";
            }
            else
            {
               std::cout << "Result: DRAW. " << wpc << "-" << bpc << "\n";;
            }
         }
      }
      
      // sort individuals based on fitness level
      std::sort( _population.begin(), _population.end() );
      
      DisplayTop( best_offset );
      
      // clone the top 50%
      Clone( best_offset );
      
      _generation++;
   }

   std::cout << "Measuring performance against a random mover 100 games:\n";
   PlayARM( 100 );

   std::cout << "Evolution Complete.\n";
}


// PlayARM
// Plays against the random mover for 'num'x2 games.
void Population::PlayARM( int num )
{
   Reversi game;
   int wpc, bpc;           // wpc/bpc = white/black piece count
   NNComputer     top_nn( false );
   RandomComputer random_mover( false );
   Reversi::PlayerHandler* wp = 0;
   Reversi::PlayerHandler* bp = 0;

   int win_count = 0;
   int fitness = 0;
   int play_count = 0;
   int piece_won = 0;
   int piece_played = 0;

   top_nn.SetNN( &_population[0] );
   for( int i=0; i<num; ++i )
   {
      // NN WHITE, Random Mover BLACK
      wpc = bpc = 0;
      top_nn.SetColor( Reversi::WHITE );
      random_mover.SetColor( Reversi::BLACK );
      wp = &top_nn;
      bp = &random_mover;
      game.Start( *wp, *bp );
      game.CountPieces( wpc, bpc );
      play_count++;
      piece_played += wpc+bpc;
      piece_won += wpc;

      std::cout << "[" << _generation << "] " << _population[0].id << "[W] vs. Random Mover[B]. ";
      if ( wpc > bpc )
      {
         win_count++;
         fitness += FITNESS_WIN;
         std::cout << "Result: " << _population[0].id << " won. " << wpc << "-" << bpc << "\n";
      }
      else if ( wpc < bpc )
      {
         fitness += FITNESS_LOSE;
         std::cout << "Result: Random Mover won. " << wpc << "-" << bpc << "\n";
      }
      else
      {
         std::cout << "Result: DRAW. " << wpc << "-" << bpc << "\n";
      }

      // swap sides
      // NN BLACK, Random Mover WHITE
      wpc = bpc = 0;
      top_nn.SetColor( Reversi::BLACK );
      random_mover.SetColor( Reversi::WHITE );
      wp = &random_mover;
      bp = &top_nn;
      game.Start( *wp, *bp );
      game.CountPieces( wpc, bpc );
      play_count++;
      piece_played += wpc+bpc;
      piece_won += bpc;
      
      std::cout << "[" << _generation << "] " << _population[0].id << "[B] vs. Random Mover[W]. ";
      if ( wpc > bpc )
      {
         fitness += FITNESS_LOSE;
         std::cout << "Result: Random Mover won. " << wpc << "-" << bpc << "\n";
      }
      else if ( wpc < bpc )
      {
         win_count++;
         fitness += FITNESS_WIN;
         std::cout << "Result: " << _population[0].id << " won. " << wpc << "-" << bpc << "\n";
      }
      else
      {
         std::cout << "Result: DRAW. " << wpc << "-" << bpc << "\n";
      }
   }

   double wcp = 100.0 * win_count / play_count;
   double pcp = 100.0 * piece_won / piece_played;
   std::cout << "Results:\n";
   std::cout << "Fitness: " << fitness << "\n";
   std::cout << "Win Count: " << win_count << "/" << play_count << "(" << wcp << ")\n";
   std::cout << "Piece Count: " << piece_won << "/" << piece_played << "(" << pcp << ")\n";
   std::cout.flush();
}


// PlayAAB
// Plays against the alpha-beta mover for 'num'x2 games.
void Population::PlayAAB( int num )
{
}


// GetSize()
// Returns the size of the population
int Population::GetSize() const
{
   return _size;
}


// GetGeneration()
// Returns the current generation of this population
int Population::GetGeneration() const
{
   return _generation;
}
