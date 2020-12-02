//
// Created by yangw on 2018-4-1.
//

#include "PCMBean.h"

PCMBean::PCMBean(SAMPLETYPE *buffer, int size) {

    this->buffer = (char *) malloc(size);
    this->buffsize = size;
    memcpy(this->buffer, buffer, size);

}

PCMBean::~PCMBean() {
    free(buffer);
    buffer = NULL;
}
