#! /usr/bin/env python
# This file is part of the dune-hdd project:
#   http://users.dune-project.org/projects/dune-hdd
# Copyright holders: Felix Schindler
# License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

from __future__ import print_function, division

import os
import sys
import pprint

from mpi4py import MPI
from dune.pymor.core import wrap_module

comm = MPI.COMM_WORLD
import mpi_cg_bindings as dune_code
from mpi_cg_bindings import MpiCGExample, Dune
import pymor.tools.mpi as pmpi

def _get_cfg(fn, extra_args=None):
    extra_args = extra_args or [''] # cannot be empty list
    try:
        assert open(fn)
        cfg = Dune.Stuff.Common.Configuration(extra_args, fn)
    except IOError as e:
        print('Could not load parameter file, defaulting')
        cfg = Dune.Stuff.Common.Configuration('grids.refinements', 2)
    return cfg


def init_example(config_file, extra_args):

    cfg = _get_cfg(config_file, extra_args)
    example = MpiCGExample(cfg.get_int("grids.refinements"))
    return example


def discretize(example, config_file):
    foo, wrapper = wrap_module(dune_code)
    pmpi.manage_object(wrapper)
    discretization = wrapper[example.discretization()]
    discretization.unlock()
    cfg = _get_cfg(config_file)
    print(cfg.report_string())
    print(config_file)
    discretization.solver_options = cfg.sub('solver')
    discretization.lock()
    return discretization


'''
def solve(mu):
    if discretization.parametric():
        # solve
        mu = Dune.Pymor.Parameter('mu', [0.1])
        print('')
        print('solving for parameter mu = {}... '.format(mu.report()), end='')
        vector = discretization.create_vector()
        discretization.solve(cfg.sub('solver'), vector, mu)
        print('done')
        filename = 'solution'
        print('writing to {}.vtu... '.format(filename), end='')
        discretization.visualize(vector, filename, 'CG_solution')
        print('done')
    else:
        print('solving ... ', end='')
        vector = discretization.create_vector()
        discretization.solve(cfg.sub('solver'), vector)
        print('done')
        filename = 'solution'
        print('writing to {}.vtu... '.format(filename), end='')
        discretization.visualize(vector, filename, 'CG_solution')
        print('done')
'''

if __name__ == '__main__':
    assert pmpi.HAVE_MPI
    if pmpi.rank0:
        import sys
        assert 1 <= len(sys.argv) <= 3
        if len(sys.argv) <= 3:
            execfile(sys.argv[1])
        else:
            try:
                import IPython
                IPython.start_kernel()  # only start a kernel since mpirun messes up the terminal
            except ImportError:
                import code
                code.interact()
    else:
        pmpi.event_loop()
