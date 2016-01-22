#ifndef DUNE_HDD_EXAMPLES_LINEARELLIPTIC_GENERIC_HH
#define DUNE_HDD_EXAMPLES_LINEARELLIPTIC_GENERIC_HH

#include <dune/common/parallel/mpihelper.hh>

#if HAVE_DUNE_FEM
# include <dune/fem/misc/mpimanager.hh>
#endif

#include <dune/stuff/common/configuration.hh>
#include <dune/stuff/common/timedlogging.hh>
#include <dune/stuff/grid/boundaryinfo.hh>
#include <dune/stuff/grid/provider.hh>
#include <dune/stuff/la/container/container-interface.hh>
#include <dune/stuff/la/solver.hh>

#include <dune/gdt/operators/projections.hh>
#include <dune/gdt/spaces/interface.hh>

#include <dune/hdd/linearelliptic/discretizations/cg.hh>
#include <dune/hdd/linearelliptic/problems.hh>


template< class GridType, Dune::GDT::ChooseSpaceBackend space_backend, Dune::Stuff::LA::ChooseBackend la_backend >
class GenericLinearellipticExample
{
  typedef typename GridType::template Codim< 0 >::Entity E;
  typedef typename GridType::ctype D;
  static const size_t d = GridType::dimension;
  typedef double R;
  static const size_t r = 1;
public:
  typedef Dune::HDD::LinearElliptic::Discretizations::CG< GridType, Dune::Stuff::Grid::ChooseLayer::leaf, R, 1, 1,
                                                          space_backend, la_backend > DiscretizationType;
  typedef typename DiscretizationType::VectorType                                     VectorType;
private:
  typedef typename DiscretizationType::MatrixType                                        MatrixType;
  typedef Dune::Stuff::GridProviders< GridType >                                         GridProvider;
  typedef Dune::Stuff::Grid::BoundaryInfoProvider< typename GridType::LeafIntersection > BoundaryProvider;
  typedef Dune::HDD::LinearElliptic::ProblemsProvider< E, D, d, R, r >                   ProblemProvider;
  typedef Dune::Stuff::LA::Solver< MatrixType >                                          SolverProvider;
public:
  static Dune::Stuff::Common::Configuration logger_options()
  {
    Dune::Stuff::Common::Configuration ret;
    ret["info"]            = "0";
    ret["debug"]           = "-1";
    ret["enable_warnings"] = "true";
    ret["enable_colors"]   = "true";
    ret["info_color"]      = Dune::Stuff::Common::TimedLogging::default_info_color();
    ret["debug_color"]     = Dune::Stuff::Common::TimedLogging::default_debug_color();
    ret["warn_color"]      = Dune::Stuff::Common::TimedLogging::default_warning_color();
    return ret;
  }

  static std::vector< std::string > grid_options()
  {
    return GridProvider::available();
  }

  static Dune::Stuff::Common::Configuration grid_options(const std::string& type)
  {
    return GridProvider::default_config(type);
  }

  static std::vector< std::string > boundary_options()
  {
    return BoundaryProvider::available();
  }

  static Dune::Stuff::Common::Configuration boundary_options(const std::string& type)
  {
    return BoundaryProvider::default_config(type);
  }

  static std::vector< std::string > problem_options()
  {
    return ProblemProvider::available();
  }

  static Dune::Stuff::Common::Configuration problem_options(const std::string& type)
  {
    return ProblemProvider::default_config(type);
  }

  static std::vector< std::string > solver_options()
  {
    return SolverProvider::types();
  }

  static Dune::Stuff::Common::Configuration solver_options(const std::string& type)
  {
    return SolverProvider::options(type);
  }

  GenericLinearellipticExample(const Dune::Stuff::Common::Configuration& logger_cfg = Dune::Stuff::Common::Configuration(),
                               const Dune::Stuff::Common::Configuration& grid_cfg = Dune::Stuff::Common::Configuration(),
                               const Dune::Stuff::Common::Configuration& boundary_cfg = Dune::Stuff::Common::Configuration(),
                               const Dune::Stuff::Common::Configuration& problem_cfg = Dune::Stuff::Common::Configuration())
    : boundary_cfg_(boundary_cfg)
  {
    try {
      int argc = 0;
      char** argv = new char* [0];
#if HAVE_DUNE_FEM
      Dune::Fem::MPIManager::initialize(argc, argv);
#else
      Dune::MPIHelper::instance(argc, argv);
#endif
    } catch (...) {}
    try {
      DSC::TimedLogger().create(logger_cfg.get("info",            logger_options().template get< ssize_t     >("info")),
                                logger_cfg.get("debug",           logger_options().template get< ssize_t     >("debug")),
                                logger_cfg.get("enable_warnings", logger_options().template get< bool        >("enable_warnings")),
                                logger_cfg.get("enable_colors",   logger_options().template get< bool        >("enable_colors")),
                                logger_cfg.get("info_color",      logger_options().template get< std::string >("info_color")),
                                logger_cfg.get("debug_color",     logger_options().template get< std::string >("debug_color")),
                                logger_cfg.get("warn_color",      logger_options().template get< std::string >("warn_color")));
    } catch (Dune::Stuff::Exceptions::you_are_using_this_wrong&) {}
    auto logger = DSC::TimedLogger().get("example.linearelliptic.generic");
    logger.info() << "creating grid (" << grid_cfg.get< std::string >("type") << ")... " << std::flush;
    grid_ = GridProvider::create(grid_cfg.get< std::string >("type"), grid_cfg);
    logger.info() << "done (has " << grid_->grid().size(0) << " elements)" << std::endl;

    logger.info() << "creating problem (" << problem_cfg.get< std::string >("type") << ")... " << std::flush;
    problem_= ProblemProvider::create(problem_cfg.get< std::string >("type"), problem_cfg);
    logger.info() << "done" << std::endl;

    logger.info() << "creating discretization... " << std::flush;
    discretization_ = DSC::make_unique< DiscretizationType >(*grid_,
                                                             boundary_cfg_,
                                                             *problem_,
                                                             -1);
//                                                             /*only_these_products=*/std::vector<std::string>({"l2", "h1", "elliptic"}));
    discretization_->init();
    logger.info() << "done (has " << discretization_->ansatz_space().mapper().size() << " DoFs)" << std::endl;
  } // GenericLinearellipticExample(...)

  DiscretizationType& discretization()
  {
    return *discretization_;
  }

  void visualize(const std::string& filename_prefix) const
  {
    if (filename_prefix.empty())
      DUNE_THROW(Dune::Stuff::Exceptions::wrong_input_given, "Given filename prefix must not be empty!");
    grid_->visualize(filename_prefix + ".grid", boundary_cfg_);
    problem_->visualize(grid_->leaf_view(), filename_prefix + ".problem", /*subsampling=*/ false);
  }

  VectorType project(const std::string& expression) const
  {
    using namespace Dune;
    if (expression.empty())
      DUNE_THROW(Stuff::Exceptions::wrong_input_given, "Given expression must not be empty!");
    auto logger = DSC::TimedLogger().get("example.linearelliptic.generic.project");
    logger.info() << "projecting '" << expression << "'... " << std::flush;
    auto discrete_function = GDT::make_discrete_function< VectorType >(discretization_->ansatz_space());
    GDT::Operators::apply_projection(Stuff::Functions::Expression< E, D, d, R, r >("x", expression), discrete_function);
    logger.info() << "done" << std::endl;
    return discrete_function.vector();
  } // ... project(...)

private:
  const Dune::Stuff::Common::Configuration boundary_cfg_;
  std::unique_ptr< Dune::Stuff::Grid::ProviderInterface< GridType > > grid_;
  std::unique_ptr< Dune::HDD::LinearElliptic::ProblemInterface< E, D, d, R, r > > problem_;
  std::unique_ptr< DiscretizationType > discretization_;

}; // class GenericLinearellipticExample


#endif // DUNE_HDD_EXAMPLES_LINEARELLIPTIC_GENERIC_HH