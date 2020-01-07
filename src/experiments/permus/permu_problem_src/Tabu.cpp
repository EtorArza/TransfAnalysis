#include "Tabu.h"
#include "Tools.h"
#include <limits>
#include "Parameters.h"



Tabu::Tabu(RandomNumberGenerator* rng, int n, int tabu_length)
{
	this->rng = rng;
	this->n = n;
	this->tabu_length = tabu_length;
	tabu_indices_i = new int[tabu_length];
	tabu_indices_j = new int[tabu_length];

	for (int k = 0; k < tabu_length; k++)
	{
		tabu_indices_i[k] = -1;
		tabu_indices_j[k] = -1;
	}

	tabu_table = new bool*[n];
	for (int i = 0; i < n; i++)
	{
		tabu_table[i] = new bool[n];
		for (int j = 0; j < n; j++)
		{
			tabu_table[i][j] = false;
		}
	}
	index_pos = 0;
}


Tabu::~Tabu()
{
	delete[] tabu_indices_i;
	delete[] tabu_indices_j;
	for (int i = 0; i < n; i++)
	{
		delete[] tabu_table[i];
	}
	delete[] tabu_table;
}


int Tabu::next_index_pos(){
	index_pos++;
	index_pos = index_pos % tabu_length;
	return index_pos;
}


void Tabu::set_tabu(int i, int j){
	if (this->tabu_coef_neat < CUTOFF_0){ // if tabu coef not high enough, do not add to tabu
		return;
	}
	if (i==-1 || j==-1)
	{
		return;
	}
	
	if (!(tabu_indices_i[index_pos] == -1))
	{
		tabu_table[tabu_indices_i[index_pos]][tabu_indices_j[index_pos]] = false;
	}
	tabu_indices_i[index_pos] = i;
	tabu_indices_j[index_pos] = j;
	tabu_table[i][j] = true;
	next_index_pos();
}

bool Tabu::is_tabu(int i, int j)
{
	// if tabu coef is near zero, return false, hence, not use tabu
	if (this->tabu_coef_neat < CUTOFF_0 && this->tabu_coef_neat > -CUTOFF_0)
	{
		return false;
	}

	if (i==-1 || j==-1)
	{
		return false;
	}

	return tabu_table[i][j];
}

void Tabu::reset()
{
	for (int k = 0; k < tabu_length; k++)
	{
		tabu_indices_i[k] = -1;
		tabu_indices_j[k] = -1;
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			tabu_table[i][j] = false;
		}
	}
	index_pos = 0;
	this->tabu_coef_neat = 0;
	}