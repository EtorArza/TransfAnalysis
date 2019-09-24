#pragma once

class RandomNumberGenerator;



class Tabu
{
public:

	Tabu(RandomNumberGenerator* rng, int n);
	~Tabu();

	void set_tabu(int i, int j);
	bool is_tabu(int i, int j);
	void reset();
	double tabu_coef_neat = 0.0;

private:
	RandomNumberGenerator* rng;
	static int n_indivs_created;
	int* tabu_indices_i;
	int* tabu_indices_j;
	bool** tabu_table;
	int index_pos = 0;
	int n;
	int next_index_pos();


};


