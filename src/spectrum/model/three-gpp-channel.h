/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering,
 * New York University
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
 * University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef THREE_GPP_CHANNEL_H
#define THREE_GPP_CHANNEL_H

#include  <complex.h>
#include "ns3/angles.h"
#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/random-variable-stream.h>
#include <ns3/boolean.h>
#include <map>

namespace ns3 {

typedef std::vector<double> doubleVector_t; //!< type definition for vectors of doubles
typedef std::vector<doubleVector_t> double2DVector_t; //!< type definition for matrices of doubles
typedef std::vector< std::complex<double> > complexVector_t; //!< type definition for complex vectors
typedef std::vector<complexVector_t> complex2DVector_t; //!< type definition for complex matrices
typedef std::vector<complex2DVector_t> complex3DVector_t; //!< type definition for complex 3D matrices

class AntennaArrayBasicModel;
class MobilityModel;

/**
 * Data structure that stores a channel realization
 */
struct ThreeGppChannelMatrix : public SimpleRefCount<ThreeGppChannelMatrix>
{
  complex3DVector_t               m_channel; //!< channel matrix H[u][s][n].
  doubleVector_t                  m_delay; //!< cluster delay.
  double2DVector_t                m_angle; //!< cluster angle angle[direction][n], where direction = 0(aoa), 1(zoa), 2(aod), 3(zod) in degree.
  double2DVector_t                m_nonSelfBlocking; //!< store the blockages
  bool                            m_isReverse; //!< true if the channel matrix was generated for the reverse link

  // TODO these are not currently used, they have to be correctly set when including the spatial consistent update procedure
  /*The following parameters are stored for spatial consistent updating*/
  Vector m_preLocUT; //!< location of UT when generating the previous channel
  Vector m_locUT; //!< location of UT
  double2DVector_t m_norRvAngles; //!< stores the normal variable for random angles angle[cluster][id] generated for equation (7.6-11)-(7.6-14), where id = 0(aoa),1(zoa),2(aod),3(zod)
  Time m_generatedTime; //!< generation time
  double m_DS; //!< delay spread
  double m_K; //!< K factor
  uint8_t m_numCluster; //!< reduced cluster number;
  double2DVector_t m_clusterPhase;
  double m_losPhase;
  bool m_los; //!< true if LOS, false if NLOS
  bool m_o2i; //!< true if O2I
  Vector m_speed; //!< velocity
  double m_dis2D; //!< 2D distance between tx and rx
  double m_dis3D; //!< 3D distance between tx and rx
};

/**
 * Data structure that stores the parameters of 3GPP TR 38.901, Table 7.5-6,
 * for a certain scenario
 */
 struct ParamsTable : public SimpleRefCount<ParamsTable>
 {
  uint8_t m_numOfCluster = 0;
  uint8_t m_raysPerCluster = 0;
  double m_uLgDS = 0;
  double m_sigLgDS = 0;
  double m_uLgASD = 0;
  double m_sigLgASD = 0;
  double m_uLgASA = 0;
  double m_sigLgASA = 0;
  double m_uLgZSA = 0;
  double m_sigLgZSA = 0;
  double m_uLgZSD = 0;
  double m_sigLgZSD = 0;
  double m_offsetZOD = 0;
  double m_cDS = 0;
  double m_cASD = 0;
  double m_cASA = 0;
  double m_cZSA = 0;
  double m_uK = 0;
  double m_sigK = 0;
  double m_rTau = 0;
  double m_shadowingStd = 0;

  double m_sqrtC[7][7];
};

/**
 * \ingroup spectrum
 *
 */
class ThreeGppChannel : public Object
{
public:
  /**
   * Constructor
   */
  ThreeGppChannel ();

  /**
   * Destructor
   */
  ~ThreeGppChannel ();

  /**
   * Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Get the parameters needed to apply the channel generation procedure
   * \param los the LOS/NLOS condition
   * \param o2i whether if it is an outdoor to indoor transmission
   * \param hBS the height of the BS
   * \param hUT the height of the UT
   * \param distance2D the 2D distance between tx and rx
   * \return the parameters table
   */
  Ptr<ParamsTable> Get3gppTable (bool los, bool o2i, double hBS, double hUT, double distance2D) const;

  /**
   * Looks for the channel matrix associated to the a,b pair in m_channelMap.
   * If found, checks if it has to be updated. If not found or if it has to
   * be updated, a new uncorrelated channel matrix is generated using the
   * method GetNewChannel.
   * \param a mobility model of the tx device
   * \param b mobility model of the rx device
   * \param txAntenna antenna of the tx device
   * \param rxAntenna antenna of the rx device
   * \param los true if the two device are in LOS, false otherwise
   * \param o2i true if outdoor-to-indoor, false otherwise
   * \return the channel matrix
   */
  Ptr<ThreeGppChannelMatrix> GetChannel (Ptr<const MobilityModel> a, Ptr<const MobilityModel> b, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna, bool los, bool o2i);

  static const uint8_t AOA_INDEX = 0; //!< index of the AOA value in the m_angle array
  static const uint8_t ZOA_INDEX = 1; //!< index of the ZOA value in the m_angle array
  static const uint8_t AOD_INDEX = 2; //!< index of the AOD value in the m_angle array
  static const uint8_t ZOD_INDEX = 3; //!< index of the ZOD value in the m_angle array

  static const uint8_t PHI_INDEX = 0; //!< index of the PHI value in the m_nonSelfBlocking array
  static const uint8_t X_INDEX = 1; //!< index of the X value in the m_nonSelfBlocking array
  static const uint8_t THETA_INDEX = 2; //!< index of the THETA value in the m_nonSelfBlocking array
  static const uint8_t Y_INDEX = 3; //!< index of the Y value in the m_nonSelfBlocking array
  static const uint8_t R_INDEX = 4; //!< index of the R value in the m_nonSelfBlocking array

 /**
  * Calculate the channel key using the Cantor function
  * \param x1 first value
  * \param x2 second value
  * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$
  */
 static constexpr uint32_t GetKey (uint32_t x1, uint32_t x2)
 {
   return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
 }

private:
  /**
   * Get the channel matrix between a and b using the procedure described in
   * 3GPP TR 38.901
   * \param locUT the location of the UT
   * \param los the LOS/NLOS condition
   * \param o2i whether if it is an outdoor to indoor transmission
   * \param txAntenna the tx antenna array
   * \param rxAntenna the rx antenna array
   * \param rxAngle the receiving angle
   * \param txAngle the transmitting angle
   * \param dis2D the 2D distance between tx and rx
   * \param hBS the height of the BS
   * \param hUT the height of the UT
   * \return the channel realization
   */
  Ptr<ThreeGppChannelMatrix> GetNewChannel (Vector locUT, bool los, bool o2i,
                                            Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna,
                                            Angles &rxAngle, Angles &txAngle,
                                            double dis2D, double hBS, double hUT) const;

  /**
   * Applies the blockage model A described in 3GPP TR 38.901
   * \param params the channel matrix
   * \param clusterAOA vector containing the azimuth angle of arrival for each cluster
   * \param clusterZOA vector containing the zenith angle of arrival for each cluster
   * \return vector containing the power attenuation for each cluster
   */
  doubleVector_t CalAttenuationOfBlockage (Ptr<ThreeGppChannelMatrix> params,
                                           doubleVector_t clusterAOA, doubleVector_t clusterZOA) const;

 /**
  * Check if the channel matrix has to be updated
  * \param channelMatrix channel matrix
  * \param los the current los condition
  * \return true if the channel matrix has to be updated, false otherwise
  */
 bool ChannelMatrixNeedsUpdate (Ptr<ThreeGppChannelMatrix> channelMatrix, bool los) const;

  std::map<uint32_t, Ptr<ThreeGppChannelMatrix>> m_channelMap; //!< map containing the channel realizations
  Time m_updatePeriod; //!< the channel update period
  double m_frequency; //!< the operating frequency
  std::string m_scenario; //!< the 3GPP scenario
  Ptr<UniformRandomVariable> m_uniformRv; //!< uniform random variable
  Ptr<NormalRandomVariable> m_normalRv; //!< normal random variable

  // parameters for the blockage model
  bool m_blockage; //!< enables the blockage model A
  uint16_t m_numNonSelfBloking; //!< number of non-self-blocking regions
  bool m_portraitMode; //!< true if potrait mode, false if landscape
  double m_blockerSpeed; //!< the blocker speed
};
} // namespace ns3

#endif /* THREE_GPP_CHANNEL_H */
