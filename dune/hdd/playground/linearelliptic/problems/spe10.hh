// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_HDD_LINEARELLIPTIC_PROBLEMS_SPE10_HH
#define DUNE_HDD_LINEARELLIPTIC_PROBLEMS_SPE10_HH

#include <memory>
#include <vector>
#include <utility>
#include <sstream>

#include <dune/common/static_assert.hh>
#include <dune/common/fmatrix.hh>

#include <dune/stuff/common/memory.hh>
#include <dune/stuff/functions/constant.hh>
#include <dune/stuff/functions/combined.hh>
#include <dune/stuff/functions/spe10.hh>
#include <dune/stuff/playground/functions/indicator.hh>

#include <dune/pymor/functions/default.hh>

#include "../../../linearelliptic/problems/default.hh"

namespace Dune {
namespace HDD {
namespace LinearElliptic {
namespace Problems {
namespace Spe10 {


template< class E, class D, int d, class R, int r >
class Model1
{
  static_assert(AlwaysFalse< E >::value, "Not available for these dimensions");
}; // class Model1


template< class EntityImp, class DomainFieldImp, class RangeFieldImp >
class Model1< EntityImp, DomainFieldImp, 2, RangeFieldImp, 1 >
  : public Problems::Default< EntityImp, DomainFieldImp, 2, RangeFieldImp, 1 >
{
  typedef Problems::Default< EntityImp, DomainFieldImp, 2, RangeFieldImp, 1 > BaseType;
  typedef Model1< EntityImp, DomainFieldImp, 2, RangeFieldImp, 1 >            ThisType;

  typedef Stuff::Functions::Constant< EntityImp, DomainFieldImp, 2, RangeFieldImp, 1 >         ConstantFunctionType;
  typedef Stuff::Functions::Indicator< EntityImp, DomainFieldImp, 2, RangeFieldImp, 1 >        IndicatorFunctionType;
  typedef Stuff::Functions::Spe10::Model1< EntityImp, DomainFieldImp, 2, RangeFieldImp, 2, 2 > Spe10FunctionType;

public:
  typedef typename BaseType::EntityType      EntityType;
  typedef typename BaseType::DomainFieldType DomainFieldType;
  static const unsigned int                  dimDomain = BaseType::dimDomain;
  typedef typename BaseType::DomainType      DomainType;
  typedef typename BaseType::RangeFieldType  RangeFieldType;
  static const unsigned int                  dimRange = BaseType::dimRange;

  static std::string static_id()
  {
    return BaseType::BaseType::static_id() + ".spe10.model1";
  }

  static Stuff::Common::Configuration default_config(const std::string sub_name = "")
  {
    std::istringstream ss("# upper part of the channel\n"
                          "channel.0.domain  = [1.95 2.00; 0.30 0.35]\n"
                          "channel.1.domain  = [2.00 2.05; 0.30 0.35]\n"
                          "channel.2.domain  = [2.05 2.10; 0.30 0.35]\n"
                          "channel.3.domain  = [2.10 2.15; 0.30 0.35]\n"
                          "channel.4.domain  = [2.15 2.20; 0.30 0.35]\n"
                          "channel.5.domain  = [2.20 2.25; 0.30 0.35]\n"
                          "channel.6.domain  = [2.25 2.30; 0.30 0.35]\n"
                          "channel.7.domain  = [2.30 2.35; 0.30 0.35]\n"
                          "channel.8.domain  = [2.35 2.40; 0.30 0.35]\n"
                          "channel.9.domain  = [2.40 2.45; 0.30 0.35]\n"
                          "channel.10.domain = [2.45 2.50; 0.30 0.35]\n"
                          "channel.11.domain = [2.50 2.55; 0.30 0.35]\n"
                          "channel.12.domain = [2.55 2.60; 0.30 0.35]\n"
                          "channel.13.domain = [2.60 2.65; 0.30 0.35]\n"
                          "channel.14.domain = [2.65 2.70; 0.30 0.35]\n"
                          "channel.15.domain = [2.70 2.75; 0.30 0.35]\n"
                          "channel.16.domain = [2.75 2.80; 0.30 0.35]\n"
                          "channel.17.domain = [2.80 2.85; 0.30 0.35]\n"
                          "channel.18.domain = [2.85 2.90; 0.30 0.35]\n"
                          "channel.19.domain = [2.90 2.95; 0.30 0.35]\n"
                          "channel.20.domain = [2.95 3.00; 0.30 0.35]\n"
                          "channel.21.domain = [3.00 3.05; 0.30 0.35]\n"
                          "channel.22.domain = [3.05 3.10; 0.30 0.35]\n"
                          "channel.23.domain = [3.10 3.15; 0.30 0.35]\n"
                          "channel.24.domain = [3.15 3.20; 0.30 0.35]\n"
                          "channel.25.domain = [3.20 3.25; 0.30 0.35]\n"
                          "channel.26.domain = [3.25 3.30; 0.30 0.35]\n"
                          "channel.0.value = 8.89427093374\n"
                          "channel.1.value = 13.6099841033\n"
                          "channel.2.value = 9.3045024096\n"
                          "channel.3.value = 9.36541962265\n"
                          "channel.4.value = 15.0913820881\n"
                          "channel.5.value = 10.7480498312\n"
                          "channel.6.value = 16.5883190883\n"
                          "channel.7.value = 31.0861602166\n"
                          "channel.8.value = 54.9712243572\n"
                          "channel.9.value = 100.952720119\n"
                          "channel.10.value = 52.0524075552\n"
                          "channel.11.value = 50.6886980657\n"
                          "channel.12.value = 480.647068351\n"
                          "channel.13.value = 124.968722073\n"
                          "channel.14.value = 253.47179089\n"
                          "channel.15.value = 59.4846423418\n"
                          "channel.16.value = 45.5966339635\n"
                          "channel.17.value = 65.4927486582\n"
                          "channel.18.value = 53.1302521008\n"
                          "channel.19.value = 356.247915509\n"
                          "channel.20.value = 119.247076023\n"
                          "channel.21.value = 428.620844564\n"
                          "channel.22.value = 105.760652121\n"
                          "channel.23.value = 225.936826992\n"
                          "channel.24.value = 86.2636769918\n"
                          "channel.25.value = 100.853603482\n"
                          "channel.26.value = 48.2026659651\n"
                          "# lower part of the channel\n"
                          "channel.27.domain = [3.00 3.05; 0.25 0.30]\n"
                          "channel.28.domain = [3.05 3.10; 0.25 0.30]\n"
                          "channel.29.domain = [3.10 3.15; 0.25 0.30]\n"
                          "channel.30.domain = [3.15 3.20; 0.25 0.30]\n"
                          "channel.31.domain = [3.20 3.25; 0.25 0.30]\n"
                          "channel.32.domain = [3.25 3.30; 0.25 0.30]\n"
                          "channel.27.value = 8.1565912417\n"
                          "channel.28.value = 10.1920122888\n"
                          "channel.29.value = 9.45165105239\n"
                          "channel.30.value = 10.8577735731\n"
                          "channel.31.value = 31.1455310916\n"
                          "channel.32.value = 20.1779587527\n"
                          "# force\n"
                          "forces.0.domain = [0.95 1.10; 0.30 0.45]\n"
                          "forces.0.value = 2\n"
                          "forces.1.domain = [3.00 3.15; 0.75 0.90]\n"
                          "forces.1.value = -1\n"
                          "forces.2.domain = [4.25 4.40; 0.25 0.40]\n"
                          "forces.2.value = -1");
    Stuff::Common::Configuration config(ss);
    config["filename"]    = Stuff::Functions::Spe10::internal::model1_filename;
    config["lower_left"]  = "[0.0 0.0]";
    config["upper_right"] = "[5.0 1.0]";
    config["parametric"] = "false";
    if (sub_name.empty())
      return config;
    else {
      Stuff::Common::Configuration tmp;
      tmp.add(config, sub_name);
      return tmp;
    }
  } // ... default_config(...)

  static std::unique_ptr< ThisType > create(const Stuff::Common::Configuration config = default_config(),
                                            const std::string sub_name = static_id())
  {
    const Stuff::Common::Configuration cfg = config.has_sub(sub_name) ? config.sub(sub_name) : config;
    const Stuff::Common::Configuration def_cfg = default_config();
    return Stuff::Common::make_unique< ThisType >(
          cfg.get("filename",    def_cfg.get< std::string >("filename")),
          cfg.get("lower_left",  def_cfg.get< DomainType >("lower_left")),
          cfg.get("upper_right", def_cfg.get< DomainType >("upper_right")),
          get_values(cfg, "channel"),
          get_values(cfg, "forces"),
          cfg.get("parametric",  def_cfg.get< bool >("parametric")));
  } // ... create(...)

  Model1(const std::string filename,
         const DomainType& lower_left,
         const DomainType& upper_right,
         const std::vector< std::tuple< DomainType, DomainType, RangeFieldType > >& channel_values = {},
         const std::vector< std::tuple< DomainType, DomainType, RangeFieldType > >& force_values = {},
         const bool parametric_channel = false)
    : BaseType(create_diffusion_factor(channel_values, parametric_channel),
               std::make_shared< Spe10FunctionType >(filename,
                                                     lower_left,
                                                     upper_right,
                                                     Stuff::Functions::Spe10::internal::model1_min_value,
                                                     Stuff::Functions::Spe10::internal::model1_max_value,
                                                     "diffusion_tensor"),
               std::make_shared< IndicatorFunctionType >(force_values, "force"),
               std::make_shared< ConstantFunctionType >(0, "dirichlet"),
               std::make_shared< ConstantFunctionType >(0, "neumann"))
  {}

private:
  typedef std::vector< std::tuple< DomainType, DomainType, RangeFieldType > > Values;

  static Values get_values(const Stuff::Common::Configuration& cfg, const std::string id)
  {
    Values values;
    DomainType tmp_lower;
    DomainType tmp_upper;
    if (cfg.has_sub(id)) {
      const Stuff::Common::Configuration sub_cfg = cfg.sub(id);
      size_t cc = 0;
      while (sub_cfg.has_sub(DSC::toString(cc))) {
        const Stuff::Common::Configuration local_cfg = sub_cfg.sub(DSC::toString(cc));
        if (local_cfg.has_key("domain") && local_cfg.has_key("value")) {
          auto domains = local_cfg.get< FieldMatrix< DomainFieldType, 2, 2 > >("domain");
          tmp_lower[0] = domains[0][0];
          tmp_lower[1] = domains[1][0];
          tmp_upper[0] = domains[0][1];
          tmp_upper[1] = domains[1][1];
          auto val = local_cfg.get< RangeFieldType >("value");
          values.emplace_back(tmp_lower, tmp_upper, val);
        } else
          break;
        ++cc;
      }
    }
    return values;
  } // ... get_values(...)

  static std::shared_ptr< typename BaseType::DiffusionFactorType > create_diffusion_factor(const Values& values,
                                                                                           const bool parametric)
  {
    auto one     = std::make_shared< ConstantFunctionType >( 1,      "one");
    auto channel = std::make_shared< IndicatorFunctionType >(values, "channel");
    if (parametric) {
      typedef Pymor::Function::AffinelyDecomposableDefault< EntityType, DomainFieldType, 2, RangeFieldType, 1 >
          ParametricType;
      auto ret = std::make_shared< ParametricType >(Stuff::Functions::make_sum(one, channel), "diffusion_factor");
      ret->register_component(channel, new Pymor::ParameterFunctional("mu", 1, "-1.0*mu"));
      return ret;
    } else {
      typedef Pymor::Function::NonparametricDefault< EntityType, DomainFieldType, 2, RangeFieldType, 1 > WrapperType;
      auto zero_pt_nine = std::make_shared< ConstantFunctionType >(0.9, "0.9");
      return std::make_shared< WrapperType >(
            Stuff::Functions::make_sum(one,
                                       Stuff::Functions::make_product(zero_pt_nine, channel, "scaled_channel"),
                                       "diffusion_factor"));
    }
  } // ... create_diffusion_factor(...)
}; // class Model1< ..., 2, ... 1 >


} // namespace Spe10
} // namespace Problems
} // namespace LinearElliptic
} // namespace HDD
} // namespace Dune

#endif // DUNE_HDD_LINEARELLIPTIC_PROBLEMS_SPE10_HH
