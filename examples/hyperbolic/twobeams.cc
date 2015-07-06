// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#include "config.h"

#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/fvector.hh>

#include <dune/stuff/common/string.hh>
#include <dune/stuff/common/profiler.hh>
#include <dune/stuff/grid/provider/cube.hh>
#include <dune/stuff/grid/information.hh>
#include <dune/stuff/la/container/common.hh>

#include <dune/gdt/discretefunction/default.hh>
#include <dune/gdt/operators/advection.hh>
#include <dune/gdt/operators/projections.hh>
#include <dune/gdt/spaces/fv/defaultproduct.hh>
#include <dune/gdt/timestepper/rungekutta.hh>

#include <dune/hdd/hyperbolic/problems/twobeams.hh>
#include <dune/hdd/hyperbolic/problems/twopulses.hh>
#include <dune/hdd/hyperbolic/problems/rectangularic.hh>
#include <dune/hdd/hyperbolic/problems/sourcebeam.hh>
#include <dune/hdd/hyperbolic/problems/onebeam.hh>

using namespace Dune::GDT;
using namespace Dune::HDD;

int main(int argc, char* argv[])
{
  typedef Dune::MPIHelper MPIHelper;
  MPIHelper::instance(argc, argv);
//  typename MPIHelper::MPICommunicator world = MPIHelper::getCommunicator();
  try {
    // setup Profiler to get timings
    DSC::Profiler& profiler_ref = DSC::profiler();
    DS::threadManager().set_max_threads(8);
    DSC_CONFIG.set("threading.partition_factor", 1, true);
    profiler_ref.startTiming("Solving");
    static const size_t dimDomain = 1;
    // for dimRange > 250, an "exceeded maximum recursive template instantiation limit" error occurs (tested with
    // clang 3.5). You need to pass -ftemplate-depth=N with N > dimRange + 10 to clang for higher dimRange.
    static const size_t dimRange = 50;
    //choose GridType
    typedef Dune::YaspGrid< dimDomain >                                     GridType;
    typedef typename GridType::Codim< 0 >::Entity                           EntityType;

    //configure Problem
    typedef Dune::HDD::Hyperbolic::Problems::TwoBeams< EntityType, double, dimDomain, double, dimRange > ProblemType;
//    typedef Dune::HDD::Hyperbolic::Problems::TwoPulses< EntityType, double, dimDomain, double, dimRange > ProblemType;
//    typedef Dune::HDD::Hyperbolic::Problems::RectangularIC< EntityType, double, dimDomain, double, dimRange > ProblemType;
//    typedef Dune::HDD::Hyperbolic::Problems::SourceBeam< EntityType, double, dimDomain, double, dimRange > ProblemType;
//    typedef Dune::HDD::Hyperbolic::Problems::OneBeam< EntityType, double, dimDomain, double, dimRange > ProblemType;
    //create Problem
    const auto problem_ptr = ProblemType::create(/*"legendre_pol.csv"*/);
    const auto& problem = *problem_ptr;

    //get grid configuration from problem
    auto grid_config = problem.grid_config();

    //get analytical flux, initial and boundary values
    typedef typename ProblemType::FluxType              AnalyticalFluxType;
    typedef typename ProblemType::SourceType            SourceType;
    typedef typename ProblemType::FunctionType          FunctionType;
    typedef typename ProblemType::BoundaryValueType     BoundaryValueType;
    typedef typename FunctionType::DomainFieldType      DomainFieldType;
    typedef typename ProblemType::RangeFieldType        RangeFieldType;
    const std::shared_ptr< const AnalyticalFluxType > analytical_flux = problem.flux();
    const std::shared_ptr< const FunctionType > initial_values = problem.initial_values();
    const std::shared_ptr< const BoundaryValueType > boundary_values = problem.boundary_values();
    const std::shared_ptr< const SourceType > source = problem.source();

    //create grid
    std::cout << "Creating Grid..." << std::endl;
    typedef Dune::Stuff::Grid::Providers::Cube< GridType >  GridProviderType;
    GridProviderType grid_provider = *(GridProviderType::create(grid_config));
    const std::shared_ptr< const GridType > grid = grid_provider.grid_ptr();

    // make a product finite volume space on the leaf grid
    std::cout << "Creating GridView..." << std::endl;
    typedef typename GridType::LeafGridView                                        GridViewType;
    const GridViewType grid_view = grid->leafGridView();
    typedef Spaces::FV::DefaultProduct< GridViewType, RangeFieldType, dimRange, 1 >   FVSpaceType;
    std::cout << "Creating FiniteVolumeSpace..." << std::endl;
    const FVSpaceType fv_space(grid_view);


    // allocate a discrete function for the concentration and another one to temporary store the update in each step
    std::cout << "Allocating discrete functions..." << std::endl;
    typedef DiscreteFunction< FVSpaceType, Dune::Stuff::LA::CommonDenseVector< RangeFieldType > > FVFunctionType;
    FVFunctionType u(fv_space, "solution");

    //project initial values
    std::cout << "Projecting initial values..." << std::endl;
    project(*initial_values, u);

    //calculate dx and choose t_end and initial dt
    std::cout << "Calculating dx..." << std::endl;
    Dune::Stuff::Grid::Dimensions< GridViewType > dimensions(fv_space.grid_view());
    const double dx = dimensions.entity_width.max();
    double dt = 0.0005; //dx/4.0;
    const double t_end = 2;
    //create operator
    typedef typename Dune::Stuff::Functions::Constant< EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange, 1 > ConstantFunctionType;
    typedef typename Dune::GDT::Operators::AdvectionGodunovWithReconstruction
            < AnalyticalFluxType, ConstantFunctionType, BoundaryValueType, FVSpaceType, Dune::GDT::Operators::SlopeLimiters::superbee > OperatorType;
    typedef typename Dune::GDT::Operators::AdvectionSource< SourceType, FVSpaceType > SourceOperatorType;

    //create butcher_array
    // forward euler
    Dune::DynamicMatrix< RangeFieldType > A(DSC::fromString< Dune::DynamicMatrix< RangeFieldType >  >("[0]"));
    Dune::DynamicVector< RangeFieldType > b(DSC::fromString< Dune::DynamicVector< RangeFieldType >  >("[1]"));
    // generic second order, x = 1 (see https://en.wikipedia.org/wiki/List_of_Runge%E2%80%93Kutta_methods)
//    Dune::DynamicMatrix< RangeFieldType > A(DSC::fromString< Dune::DynamicMatrix< RangeFieldType >  >("[0 0; 1 0]"));
//    Dune::DynamicVector< RangeFieldType > b(DSC::fromString< Dune::DynamicVector< RangeFieldType >  >("[0.5 0.5]"));
    // classic fourth order RK
//    Dune::DynamicMatrix< RangeFieldType > A(DSC::fromString< Dune::DynamicMatrix< RangeFieldType >  >("[0 0 0 0; 0.5 0 0 0; 0 0.5 0 0; 0 0 1 0]"));
//    Dune::DynamicVector< RangeFieldType > b(DSC::fromString< Dune::DynamicVector< RangeFieldType >  >("[" + DSC::toString(1.0/6.0) + " " + DSC::toString(1.0/3.0) + " " + DSC::toString(1.0/3.0) + " " + DSC::toString(1.0/6.0) + "]"));

    //search suitable time step length
//    std::pair< bool, double > dtpair = std::make_pair(bool(false), dt);
//    while (!(dtpair.first)) {
//      ConstantFunctionType dx_function(dx);
//      OperatorType advection_operator(*analytical_flux, dx_function, dt, *boundary_values, fv_space, true);
//      Dune::GDT::TimeStepper::RungeKutta< OperatorType, FVFunctionType, SourceType > timestepper(advection_operator, u, *source, dx, A, b);
//      dtpair = timestepper.find_suitable_dt(dt, 2, 500, 1000);
//      dt = dtpair.second;
//    }
    std::cout <<" dt/dx: "<< dt/dx << std::endl;

    //create timestepper
    std::cout << "Creating TimeStepper..." << std::endl;
    ConstantFunctionType dx_function(dx);
    OperatorType advection_operator(*analytical_flux, dx_function, dt, *boundary_values, fv_space, true);
    SourceOperatorType source_operator(*source, fv_space);
    Dune::GDT::TimeStepper::RungeKutta< OperatorType, SourceOperatorType, FVFunctionType, double > timestepper(advection_operator, source_operator, u, dx, A, b);

    const double saveInterval = t_end/1000 > dt ? t_end/1000 : dt;

    // now do the time steps
    timestepper.solve(t_end, dt, saveInterval);
    profiler_ref.stopTiming("Solving");
    std::cout << "Solving done, took " << profiler_ref.getTiming("Solving", false)/1000.0 << " seconds (walltime "
                 << profiler_ref.getTiming("Solving", true)/1000.0 << " seconds)" << std::endl;

    // visualize solution
    timestepper.visualize_solution();
    // write solution to *.csv file
    std::cout << "Writing solution to .csv file...";
    remove((problem.static_id() + "_P" + DSC::toString(dimRange - 1) + "CGLegendre.csv").c_str());
    std::ofstream output_file(problem.static_id() + "_P" + DSC::toString(dimRange - 1) + "CGLegendre.csv");
    const auto solution = timestepper.solution();
    // write first line
    const auto it_end_2 = grid_view.template end< 0 >();
    for (auto it = grid_view.template begin< 0 >(); it != it_end_2; ++it) {
      const auto& entity = *it;
      output_file << ", " << DSC::toString(entity.geometry().center()[0]);
    }
    output_file << std::endl;
    for (auto& pair : solution) {
      output_file << DSC::toString(pair.first);
      const auto discrete_func = pair.second;
      for (auto it = grid_view.template begin< 0 >(); it != it_end_2; ++it) {
        const auto& entity = *it;
        output_file << ", " << DSC::toString(discrete_func.local_discrete_function(entity)->evaluate(entity.geometry().local(entity.geometry().center()))[0]);
      }
      output_file << std::endl;
    }
    output_file.close();
    std::cout << " done" << std::endl;


    return 0;
  } catch (Dune::Exception& e) {
    std::cerr << "Dune reported: " << e.what() << std::endl;
    std::abort();
  }
} // ... main(...)
