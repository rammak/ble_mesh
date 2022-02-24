/*
 * routing_table.h
 *
 *  Created on: Dec. 20, 2021
 *      Author: rutwij
 */

#ifndef MESH_ROUTING_TABLE_H_
#define MESH_ROUTING_TABLE_H_

#include <math.h>
#include <mesh_config.h>
#include <sys/_stdint.h>

struct __attribute__((__packed__)) route {
  unsigned int distance : 2;
};

//struct route route_table[64];

#endif /* MESH_ROUTING_TABLE_H_ */
