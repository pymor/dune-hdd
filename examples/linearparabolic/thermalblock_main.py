#!/usr/bin/env python2
#
# This file is part of the dune-hdd project:
#   https://github.com/pymor/dune-hdd
# Copyright Holders: Felix Schindler
# License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

from __future__ import division, print_function

import pymor.core
from pymor.core.logger import getLogger

from thermalblock_prepare import prepare


config = {'dune_num_blocks': '[2 2 1]',
          'dune_num_grid_elements': '[512 512 512]',
          'dune_products': ['l2', 'h1', 'elliptic'],
          'dune_log_info_level': 99,
          'dune_log_debug_level': -1,
          'dune_log_enable_warnings': True,
          'dune_example': 'CG_Thermalblock_1dYaspGrid_fem_eigen',
          'parameter_range': (0.1, 10),
          'initial_values_expr': 'exp(-((x[0]-0.5)*(x[0]-0.5))/(2*0.1*0.1))',
          'end_time': 0.075,
          'num_timesteps': 250}

pymor.core.logger.MAX_HIERACHY_LEVEL = 2
getLogger('pymor.algorithms').setLevel('INFO')
getLogger('pymor.la.genericsolvers.lgmres').setLevel('WARN')
getLogger('pymor.WrappedDiscretization').setLevel('WARN')
getLogger('dune.pymor.discretizations').setLevel('WARN')


if __name__ == '__main__':

    detailed_data = prepare(config)
    print('')

    discretization = detailed_data['discretization']
    u = discretization.solve([1, 1])
    discretization.visualize(u, filename='temperature__diffusion_1')
    u = discretization.solve([0.1, 10])
    discretization.visualize(u, filename='temperature__diffusion_01_10')
