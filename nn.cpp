#include "lib.h"
#include "nn.h"
#include "common.h"

inline double sigmoid( double f )
{
   return 1.0/(1.0+exp(-f));
}

inline double sigmoid_last( double f )
{
   return 1.0/(1.0+exp(-f/8.0));
}

NeuralNetwork::NeuralNetwork()
{
   _nodes.clear();
   _weights.clear();
   _layer_info.clear();
   _layer_count = 0;
   _nodes_count = 0;
   _weight_count = 0;
}

void NeuralNetwork::Create( const char* info )
{
   std::stringstream ss(info);
   int x; int t = 0;
   _layer_info.clear();
   _layer_count = 0;
   _nodes_count = 0;
   _weight_count = 0;
   // counts layer, nodes and weights
   while( !ss.eof() )
   {
      ss >> x; _layer_info.push_back(x);
      _nodes_count += x;
      if ( t != 0 )
      {
         t *= x;
         _weight_count += t;
      }
      t = x;
      _layer_count++;
   }

   // create nodes
   _nodes.resize( _nodes_count );
   for( int i=0; i<_nodes_count; ++i )
      _nodes[i] = 0.0f;

   // create weights
   _weights.resize( _weight_count );
   int pwc = 0; int pl = 0;   // pwc = previous layer weight count, pl = node count in the previous layer
   int plx = 0;               // plx = previous layer node count including current layer 'l'
   for( int l=0; l<_layer_count-1; ++l )           // for each layer 'l'
   {
      int wtl = _layer_info[l] * _layer_info[l+1]; // wtl = weights at this layer
      plx += _layer_info[l];
      for ( int w=0; w<wtl; ++w )
      {
         _weights[pwc+w].weight = randf(-0.5,0.5);
         _weights[pwc+w].src = pl + w/_layer_info[l+1];
         _weights[pwc+w].dest = plx + w%_layer_info[l+1];
      }
      pwc += wtl;
      pl += _layer_info[l];
   }
}

void NeuralNetwork::Create( const char* info, const NeuralNetwork::weight_type& wn )
{
   std::stringstream ss(info);
   int x; int t = 0;
   _layer_info.clear();
   _layer_count = 0;
   _nodes_count = 0;
   _weight_count = 0;
   // counts layer, nodes and weights
   while( !ss.eof() )
   {
      ss >> x; _layer_info.push_back(x);
      _nodes_count += x;
      if ( t != 0 )
      {
         t *= x;
         _weight_count += t;
      }
      t = x;
      _layer_count++;
   }

   // create nodes
   _nodes.resize( _nodes_count );
   for( int i=0; i<_nodes_count; ++i )
      _nodes[i] = 0.0f;

   // create weights
   _weights.resize( _weight_count );
   int pwc = 0; int pl = 0;   // pwc = previous layer weight count, pl = node count in the previous layer
   int plx = 0;               // plx = previous layer node count including current layer 'l'
   for( int l=0; l<_layer_count-1; ++l )           // for each layer 'l'
   {
      int wtl = _layer_info[l] * _layer_info[l+1]; // wtl = #weights at this layer
      plx += _layer_info[l];
      for ( int w=0; w<wtl; ++w )
      {
         _weights[pwc+w].weight = wn[pwc+w].weight;
         _weights[pwc+w].src = pl + w/_layer_info[l+1];
         _weights[pwc+w].dest = plx + w%_layer_info[l+1];
      }
      pwc += wtl;
      pl += _layer_info[l];
   }
}

void NeuralNetwork::ReplaceWeight( const NeuralNetwork::weight_type& w )
{
   for( int i=0; i<_weight_count; ++i )
      _weights[i].weight = w[i].weight;
}

void NeuralNetwork::Input( NeuralNetwork::nodes_type& input )
{
   // clear previous results
   for( int i=0; i<_nodes_count; ++i )
      _nodes[i] = 0.0f;

   if ( _layer_info.size() > 0 )
   {
      int val = ( _layer_info[0] < int(input.size()) ) ? _layer_info[0] : int(input.size());
      for( int i=0; i<val; ++i ) _nodes[i] = input[i];
   }
}

void NeuralNetwork::FeedForward()
{
   _link* t = 0; int li=0; int ls = _layer_info[li];
   for( int i=0; i<_weight_count; ++i )
   {
      t = &_weights[i];
      // call sigmoid() when certain indices are reached
      if ( t->src == ls )
      {
         ++li;
         ls += _layer_info[li];
         for( int j=t->src; j<ls; ++j )
            _nodes[j] = sigmoid(_nodes[j]);
      }
      _nodes[t->dest] += _nodes[t->src] * t->weight;
   }

   _nodes[_nodes_count-1] = sigmoid_last(_nodes[_nodes_count-1]);
}

const NeuralNetwork::nodes_type& NeuralNetwork::GetNodes() const
{
   return _nodes;
}

const NeuralNetwork::weight_type& NeuralNetwork::GetWeights() const
{
   return _weights;
}

const NeuralNetwork::layer_info_type& NeuralNetwork::GetLayerInfo() const
{
   return _layer_info;
}

NeuralNetwork::value_type NeuralNetwork::GetOutput() const
{
   return _nodes[_nodes_count-1];
}

int NeuralNetwork::LayerCount() const
{
   return _layer_count;
}

int NeuralNetwork::HiddenLayerCount() const
{
   return _layer_count-2;
}

int NeuralNetwork::InputNodesCount() const
{
   return _layer_info[0];
}

int NeuralNetwork::OutputNodesCount() const
{
   return _layer_info[_layer_count-1];
}

int NeuralNetwork::NodesCount() const
{
   return _nodes_count;
}

int NeuralNetwork::WeightCount() const
{
   return _weight_count;
}
