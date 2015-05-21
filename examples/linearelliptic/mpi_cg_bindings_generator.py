#! /usr/bin/env python
# This file is part of the dune-hdd project:
#   http://users.dune-project.org/projects/dune-hdd
# Copyright holders: Felix Schindler
# License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

import sys
from pybindgen import param, retval

from dune.pymor.core import prepare_python_bindings, inject_lib_dune_pymor, finalize_python_bindings
from dune.pymor.discretizations import inject_StationaryDiscretizationImplementation


def inject_Example(module, exceptions, interfaces, CONFIG_H):
    '''injects the user code into the module'''
    # first the discretization
    gridtype = 'Dune::SPGrid< double, 2 >'
    gridlayertype = 'Dune::Stuff::Grid::ChooseLayer::leaf'
    rangefieldtype = 'double'
    matrixtype = 'Dune::Stuff::LA::IstlRowMajorSparseMatrix< ' + rangefieldtype + ' >'
    vectortype = 'Dune::Stuff::LA::IstlDenseVector< ' + rangefieldtype + ' >'
    discretizationname = 'Dune::HDD::LinearElliptic::Discretizations::MpiCG'
    discretizationfullname = discretizationname

    view_type = 'Dune::GridView< SPGridViewTraits< const {}, Dune::All_Partition > >'.format(gridtype)
    space_type = 'Dune::GDT::Spaces::CG:PdelabBased< {}, 1, double >'.format(view_type)
    operator_type = 'Dune::Pymor::Operators::LinearAffinelyDecomposedContainerBased<{},{},{} >'.format(matrixtype, vectortype, space_type)
    discretization = inject_StationaryDiscretizationImplementation(
        module, exceptions, interfaces, CONFIG_H,
        discretizationname,
        Traits={'VectorType': vectortype,
                'OperatorType': operator_type,
                'FunctionalType': 'Dune::Pymor::Functionals::LinearAffinelyDecomposedVectorBased< {} >'.format(vectortype),
                'ProductType': operator_type}
        )
    # then add the example
    mpicgexample = module.add_class('MpiCGExample',
                                    custom_name='MpiCGExample')
    mpicgexample.add_method('static_id',
                            retval('std::string'),
                            [], is_const=True, throw=exceptions)
    mpicgexample.add_constructor([], throw=exceptions)
    mpicgexample.add_method('discretization_and_return_ptr',
                            retval(discretizationfullname + ' *', caller_owns_return=True),
                            [], is_const=True, throw=exceptions,
                            custom_name='discretization')

if __name__ == '__main__':
    # prepare the module
    module, pybindgen_filename, config_h_filename = prepare_python_bindings(sys.argv[1:])
    # add all of libdunepymor
    module, exceptions, interfaces, CONFIG_H = inject_lib_dune_pymor(module, config_h_filename)
    # add example user code (see above)
    inject_Example(module, exceptions, interfaces, CONFIG_H)
    # and finally write the pybindgen .cc file
    finalize_python_bindings(module, pybindgen_filename)
