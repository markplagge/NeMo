//
// Created by Mark Plagge on 8/2/20.
//
#include "IOStack.h"

char *stat_filename_base = "energy_count_rank_";
FILE *out_file;
char stat_filename_computed[1024];

double manhattan_distance(double x1,  double x2, double y1, double y2){
  double distance;
  int x_dif, y_dif;

  x_dif = x2 - x1;
  y_dif = y2 - y1;
  if(x_dif < 0)
    x_dif = -x_dif;
  if(y_dif < 0)
    y_dif = -y_dif;
  distance = x_dif + y_dif;
  return distance;
}

double compute_network_distance(int source_core, int dest_core){
  //grid:
  // 0,1,2..15
  // 17,18,19..31
  int source_x = source_core % 16;
  int source_y = source_core / 16;
  int dest_x = source_core % 16;
  int dest_y = source_core / 16;

  double distance = manhattan_distance(source_x, dest_x, source_y, dest_y);
  return distance;
}

void write_header(){
  fputs("source_core, source_neuron, num_sops, num_rng, num_spikes, "
        "dest_core, network_distance\n",out_file);
}
void write_line(tn_energy *energy_data){
  double distance = compute_network_distance(energy_data->my_core,
      energy_data->dest_core);
  int num_sops = energy_data->sops_count;
  int num_rng = energy_data->rng_count;

  fprintf(out_file,"%i,%i,%llu,%llu,%llu,%d,%f\n",energy_data->my_core,
          energy_data->my_neuron, energy_data->sops_count,
          energy_data->rng_count, energy_data->spike_count,
          energy_data->dest_core, distance);
}

void save_energy_stats(tn_energy *energy_data, int source_rank){
  static bool is_file_init = false;
  if(is_file_init == false){
    sprintf(stat_filename_computed,"%s%i.csv", stat_filename_base,source_rank);
    out_file=fopen(stat_filename_computed,"w");
    write_header();
    is_file_init = true;
  }

  write_line(energy_data);


}

void close_energy_stat_file(){
  fclose(out_file);
}