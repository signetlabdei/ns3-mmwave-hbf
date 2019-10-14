/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (C) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#ifndef ANTENNA_ARRAY_BASIC_H_
#define ANTENNA_ARRAY_BASIC_H_

#include <ns3/antenna-model.h>
#include <complex>
#include <ns3/nstime.h>
#include <ns3/spectrum-model.h>

namespace ns3 {

class NetDevice;

/**
 * \ingroup antenna
 * \brief The AntennaArrayBasicModel class
 *
 * The class provides a basic interface for any antenna that uses Beams.
 */
class AntennaArrayBasicModel : public AntennaModel
{
public:
  /**
   * Constructor
   */
  AntennaArrayBasicModel ();
  /**
   * Destructor
   */
  virtual ~AntennaArrayBasicModel ();
  /**
   * GetTypeId
   * \return the TypeId of this instance
   */
  static TypeId GetTypeId ();

  /**
   * Syntax sugar to express a vector of complex
   */
  typedef std::vector<std::complex<double>> complexVector_t;
  /**
   * Representation of a beam id
   *
   * This ID usually come with the real physical representation
   * of a Beam, expressed by BeamformingVector.
   */
  typedef uint32_t BeamId;
  /**
   * Physical representation of a beam.
   *
   * Contains the vector of the antenna weight, as well as the beam id. These
   * values are stored as std::pair, and we provide utility functions to
   * extract them.
   *
   * \see GetVector
   * \see GetBeamId
   */
  typedef std::pair<complexVector_t, BeamId>  BeamformingVector;

  /**
   * Get weight vector from a BeamformingVector
   * \param v the BeamformingVector
   * \return the weight vector
   */
  static complexVector_t GetVector (BeamformingVector v)
  {
    return v.first;
  }
  /**
   * Extract the beam id from the beamforming vector specified
   * \return the beam id
   * \param v the beamforming vector
   */
  static BeamId GetBeamId (const BeamformingVector &v)
  {
    return v.second;
  }

  /**
    * This method in inherited from the AntennaModel. It is
    * designed to return the power gain in dBi of the antenna
    * radiation pattern at the specified angles;
    * dBi means dB with respect to the gain of an isotropic radiator.
    * Since a power gain is used, the efficiency of
    * the antenna is expected to be included in the gain value.
    *
    * \param a the spherical angles at which the radiation pattern should
    * be evaluated
    *
    * \return the power gain in dBi
    */
  virtual double GetGainDb (Angles a) = 0;

  /**
   * This function sets the beamforming weights of the antenna
   * for transmission or reception to/from a specified connected device
   * using the beam that is specified by the beamId
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
  virtual void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                     Ptr<NetDevice> device) = 0 ;

  /**
   * Change the beamforming vector for a device
   * \param device Device to change the beamforming vector for
   */
  virtual void ChangeBeamformingVector (Ptr<NetDevice> device) = 0 ;

  /**
   * Change the antenna model to omnidirectional (ignoring the beams)
   */
  virtual void ChangeToOmniTx () = 0;

  /**
   * This function that returns the beamforming vector that is currently being
   * used by the antenna.
   * \return the current beamforming vector
   */
  virtual BeamformingVector GetCurrentBeamformingVector () = 0;

  /**
   * This function that returns the beamforming vector weights that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual BeamformingVector GetBeamformingVector (Ptr<NetDevice> device) = 0;

  /**
   * This function that returns the last time at which the beamforming vector was updated
   * \param device the device to which is connected the device of this antenna array
   * \return the last time at which the beamforming vector has been updated
   */
  virtual Time GetBeamformingVectorUpdateTime (Ptr<NetDevice> device) = 0;

  /**
   * Returns a bool that says if the current transmission is configured to be
   * omni.
   * \return whether the transmission is set to omni
   */
  virtual bool IsOmniTx () = 0;

  /**
   * This function returns the radiation pattern for the specified vertical
   * and the horizontal angle.
   * \param vangle vertical angle
   * \param hangle horizontal angle
   * \return returns the radiation pattern
   */
  virtual double GetRadiationPattern (double vangle, double hangle) = 0;

  /**
   * This function returns the location of the antenna element inside of the
   * sector assuming the left bottom corner is (0,0,0).
   * \param index index of the antenna element
   * \return returns the 3D vector that represents the position of the antenna
   * by specifing x, y and z coordinate
   */
  virtual Vector GetAntennaLocation (uint8_t index) = 0;

  /**
   * Returns the number of antenna elements in the first dimension.
   */
  virtual uint8_t GetAntennaNumDim1 () const = 0;

  /**
   * Returns the number of antenna elements in the second dimension.
   */
  virtual uint8_t GetAntennaNumDim2 () const = 0;

  /**
   * Set the number of antenna elements in the first dimension
   * \param antennaNum the number of antenna elements in the first dimension
   */
  virtual void SetAntennaNumDim1 (uint8_t antennaNum) = 0;

  /**
   * Set the number of antenna elements in the second dimension
   * \param antennaNum the number of antenna elements in the second dimension
   */
  virtual void SetAntennaNumDim2 (uint8_t antennaNum) = 0;

};

} /* namespace ns3 */

#endif /* SRC_ANTENNA_ARRAY_BASIC_H_ */
