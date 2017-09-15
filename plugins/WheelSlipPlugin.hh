/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef GAZEBO_WHEEL_SLIP_PLUGIN_HH_
#define GAZEBO_WHEEL_SLIP_PLUGIN_HH_

#include <memory>

#include <gazebo/common/Plugin.hh>
#include <gazebo/util/system.hh>

namespace gazebo
{
  // Forward declare private data class
  class WheelSlipPluginPrivate;

  /// \brief A plugin that updates ODE wheel slip parameters based
  /// on wheel spin velocity (radius * spin rate).
  /// It currently assumes that the fdir1 friction parameter is set
  /// parallel to the joint axis (often [0 0 1]) and that the link
  /// origin is on the joint axis.
  /// The ODE slip parameter is documented as Force-Dependent Slip
  /// (slip1, slip2) in the ODE user guide:
  /// http://ode.org/ode-latest-userguide.html#sec_7_3_7
  /// and it has units of velocity / force (m / s / N),
  /// similar to the inverse of a viscous damping coefficient.
  /// The slip_compliance parameters specified in this plugin
  /// have units of 1/force (1/N), and the wheel spin velocity
  /// is multiplied by these compliances at each time step
  /// to provide a scaled form of slip, that matches how
  /// slip is often defined for wheel-terrain interaction models.
  ///
  /** \verbatim
    <plugin filename="libWheelSlipPlugin.so" name="wheel_slip">
      <wheel link_name="wheel_front_left">
        <slip_compliance_lateral>0</slip_compliance_lateral>
        <slip_compliance_longitudinal>0.1</slip_compliance_longitudinal>
      </wheel>
      <wheel link_name="wheel_front_right">
        <slip_compliance_lateral>0</slip_compliance_lateral>
        <slip_compliance_longitudinal>0.1</slip_compliance_longitudinal>
      </wheel>
      <wheel link_name="wheel_rear_left">
        <slip_compliance_lateral>0</slip_compliance_lateral>
        <slip_compliance_longitudinal>0.1</slip_compliance_longitudinal>
      </wheel>
      <wheel link_name="wheel_rear_right">
        <slip_compliance_lateral>0</slip_compliance_lateral>
        <slip_compliance_longitudinal>0.1</slip_compliance_longitudinal>
      </wheel>
    </plugin>
   \endverbatim */
  class GAZEBO_VISIBLE WheelSlipPlugin : public ModelPlugin
  {
    /// \brief Constructor.
    public: WheelSlipPlugin();

    /// \brief Destructor.
    public: virtual ~WheelSlipPlugin();

    // Documentation inherited
    public: virtual void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);

    /// \brief Get parent model.
    /// \return pointer to parent model.
    public: physics::ModelPtr GetParentModel() const;

    /// \brief Set unitless lateral slip compliance for all wheels.
    /// \param[in] _compliance unitless slip compliance to set.
    public: void SetSlipComplianceLateral(const double _compliance);

    /// \brief Set unitless longitudinal slip compliance for all wheels.
    /// \param[in] _compliance unitless slip compliance to set.
    public: void SetSlipComplianceLongitudinal(const double _compliance);

    /// \brief Update the plugin. This is updated every iteration of
    /// simulation.
    private: void Update();

    /// \brief Private data pointer.
    private: std::unique_ptr<WheelSlipPluginPrivate> dataPtr;
  };
}
#endif
