/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
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

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/test.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/angles.h"
#include "ns3/node-container.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/antenna-array-model.h"
#include "ns3/three-gpp-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/channel-condition-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/wifi-spectrum-value-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThreeGppChannelTestSuite");

/**
 * \ingroup spectrum
 *
 * Test case for the ThreeGppChannel class.
 * 1) check if the channel matrix has the correct dimensions
 * 2) check if the Frobenius norm of the channel matrix is equal to the
 *    product between the number of tx and rx antenna elements
 * 3) check if GetChannel returns the same channel matrix both for direct
 *    and the reverse link
 * 4) check if the channel matrix is correctly updated
 */
class ThreeGppChannelTest : public TestCase
{
public:
  /**
   * Constructor
   */
  ThreeGppChannelTest ();

  /**
   * Destructor
   */
  virtual ~ThreeGppChannelTest ();

private:
  /**
   * Build the test scenario
   */
  virtual void DoRun (void);

  /**
   * Test if the channel matrix is correctly generated
   * \param channelModel the ThreeGppChannel object used to generate the channel matrix
   * \param txMob the mobility model of the first node
   * \param rxMob the mobility model of the second node
   * \param txAntenna the antenna object associated to the first node
   * \param rxAntenna the antenna object associated to the second node
   */
  void CheckChannelMatrix (Ptr<ThreeGppChannel> channelModel, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna);

  /**
   * Test if the channel matrix is correcly updated
   * \param channelModel the ThreeGppChannel object used to generate the channel matrix
   * \param txMob the mobility model of the first node
   * \param rxMob the mobility model of the second node
   * \param txAntenna the antenna object associated to the first node
   * \param rxAntenna the antenna object associated to the second node
   */
  void CheckChannelUpdate (Ptr<ThreeGppChannel> channelModel, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna);

  /**
   * This method is used by CheckChannelUpdate to schedule the channel matrix
   * computation at different time instants
   * \param channelModel the ThreeGppChannel object used to generate the channel matrix
   * \param txMob the mobility model of the first node
   * \param rxMob the mobility model of the second node
   * \param txAntenna the antenna object associated to the first node
   * \param rxAntenna the antenna object associated to the second node
   * \param update whether if the channel matrix should be updated or not
   */
  void DoGetChannel (Ptr<ThreeGppChannel> channelModel, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna, bool update);

  Ptr<ThreeGppChannelMatrix> m_currentChannel; //!< used by DoGetChannel to store the current channel matrix
};

ThreeGppChannelTest::ThreeGppChannelTest ()
  : TestCase ("Test case for the ThreeGppChannel class")
{
}

ThreeGppChannelTest::~ThreeGppChannelTest ()
{
}

void
ThreeGppChannelTest::CheckChannelMatrix (Ptr<ThreeGppChannel> channelModel, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna)
{
  uint32_t txAntennaElements[] {txAntenna->GetAntennaNumDim1 (), txAntenna->GetAntennaNumDim2 ()};
  uint32_t rxAntennaElements[] {rxAntenna->GetAntennaNumDim1 (), rxAntenna->GetAntennaNumDim2 ()};

  // generate the channel matrix
  Ptr<ThreeGppChannelMatrix> channelMatrix = channelModel->GetChannel (txMob, rxMob, txAntenna, rxAntenna, true, false);

  // check the channel matrix dimensions
  NS_TEST_ASSERT_MSG_EQ (channelMatrix->m_channel.at (0).size (), txAntennaElements[0] * txAntennaElements[1], "The second dimension of H should be equal to the number of tx antenna elements");
  NS_TEST_ASSERT_MSG_EQ (channelMatrix->m_channel.size (), rxAntennaElements[0] * rxAntennaElements[1], "The first dimension of H should be equal to the number of rx antenna elements");

  // check the matrix norm
  uint8_t numCluster = channelMatrix->m_channel.at (0).at (0).size ();

  double norm = 0;
  for (uint32_t uIndex = 0; uIndex < rxAntennaElements[0] * rxAntennaElements[1]; uIndex++)
  {
    for (uint32_t sIndex = 0; sIndex < txAntennaElements[0] * txAntennaElements[1]; sIndex++)
    {
      std::complex<double> h_tot;
      for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
      {
        h_tot += channelMatrix->m_channel.at (uIndex).at (sIndex).at (cIndex);
      }
      norm += std::abs (h_tot);
    }
  }

  NS_TEST_ASSERT_MSG_EQ_TOL (norm, rxAntennaElements[0] * rxAntennaElements[1] * txAntennaElements[0] * txAntennaElements[1], 0.5, "The Frobenius norm of H should be equal to M*N");

  // check channel reciprocity. The channel matrix for the a->b link should be
  // equal to the channel matrix for the a<-b case
  Ptr<ThreeGppChannelMatrix> channelMatrixReverse = channelModel->GetChannel (rxMob, txMob, rxAntenna, txAntenna, true, false);

  bool check = true;
  for (uint32_t uIndex = 0; uIndex < rxAntennaElements[0] * rxAntennaElements[1]; uIndex++)
  {
    for (uint32_t sIndex = 0; sIndex < txAntennaElements[0] * txAntennaElements[1]; sIndex++)
    {
      std::complex<double> h_tot;
      for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
      {
        check = (channelMatrix->m_channel.at (uIndex).at (sIndex).at (cIndex) == channelMatrixReverse->m_channel.at (uIndex).at (sIndex).at (cIndex));
      }
    }
  }

  NS_TEST_ASSERT_MSG_EQ (check,  true, "The channel matrix for the reverse link is different");

  // check if the reverse indicator is correctly set
  NS_TEST_ASSERT_MSG_EQ (channelMatrixReverse->m_isReverse, true, "The m_isReverse indicator is not correcly set");
}

void
ThreeGppChannelTest::CheckChannelUpdate (Ptr<ThreeGppChannel> channelModel, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna)
{
  // check if the channel matrix is correctly updated
  channelModel->SetAttribute ("UpdatePeriod", TimeValue (MilliSeconds (100)));

  // compute the channel matrix for the first time
  Simulator::Schedule (MilliSeconds (1), &ThreeGppChannelTest::DoGetChannel, this, channelModel, txMob, rxMob, txAntenna, rxAntenna, true);

  // call GetChannel before the update period is exceeded, the channel matrix
  // should not be updated
  Simulator::Schedule (MilliSeconds (51), &ThreeGppChannelTest::DoGetChannel, this, channelModel, txMob, rxMob, txAntenna, rxAntenna, false);

  // call GetChannel when the update period is exceeded, the channel matrix
  // should be recomputed
  Simulator::Schedule (MilliSeconds (101), &ThreeGppChannelTest::DoGetChannel, this, channelModel, txMob, rxMob, txAntenna, rxAntenna, true);

}

void
ThreeGppChannelTest::DoGetChannel (Ptr<ThreeGppChannel> channelModel, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<AntennaArrayBasicModel> txAntenna, Ptr<AntennaArrayBasicModel> rxAntenna, bool update)
{
  // retrieve the channel matrix
  Ptr<ThreeGppChannelMatrix> channelMatrix = channelModel->GetChannel (txMob, rxMob, txAntenna, rxAntenna, true, false);

  if (m_currentChannel == 0)
  {
    // this is the first time we compute the channel matrix, we initialize
    // m_currentChannel
    m_currentChannel = channelMatrix;
  }
  else
  {
    // compare the old and the new channel matrices
    NS_TEST_ASSERT_MSG_EQ ((m_currentChannel != channelMatrix),  update, "The channel matrix is not correctly updated");
  }
}

void
ThreeGppChannelTest::DoRun (void)
{
  // Build the scenario for the test

  uint8_t txAntennaElements[] {2, 2}; // tx antenna dimensions
  uint8_t rxAntennaElements[] {4, 4}; // rx antenna dimensions

  // create the ThreeGppChannel object used to generate the channel matrix
  Ptr<ThreeGppChannel> channelModel = CreateObject<ThreeGppChannel> ();
  channelModel->SetAttribute ("Frequency", DoubleValue (60.0e9));
  channelModel->SetAttribute ("Scenario", StringValue ("UMa"));

  // create the tx and rx nodes
  NodeContainer nodes;
  nodes.Create (2);

  // create the tx and rx devices
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();

  // associate the nodes and the devices
  nodes.Get (0)->AddDevice (txDev);
  txDev->SetNode (nodes.Get (0));
  nodes.Get (1)->AddDevice (rxDev);
  rxDev->SetNode (nodes.Get (1));

  // create the tx and rx mobility models and set their positions
  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel> ();
  txMob->SetPosition (Vector (0.0,0.0,10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel> ();
  rxMob->SetPosition (Vector (100.0,0.0,1.6));

  // associate the nodes and the mobility models
  nodes.Get (0)->AggregateObject (txMob);
  nodes.Get (1)->AggregateObject (rxMob);

  // create the tx and rx antennas and set the their dimensions
  Ptr<AntennaArrayModel> txAntenna = CreateObject<AntennaArrayModel> ();
  txAntenna->SetAntennaNumDim1 (txAntennaElements [0]);
  txAntenna->SetAntennaNumDim2 (txAntennaElements [1]);

  Ptr<AntennaArrayModel> rxAntenna = CreateObject<AntennaArrayModel> ();
  rxAntenna->SetAntennaNumDim1 (rxAntennaElements [0]);
  rxAntenna->SetAntennaNumDim2 (rxAntennaElements [1]);

  // test if the channel matrix is correctly generated
  CheckChannelMatrix (channelModel, txMob, rxMob, txAntenna, rxAntenna);

  // test if the channel matrix is correctly updated
  CheckChannelUpdate (channelModel, txMob, rxMob, txAntenna, rxAntenna);

  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup spectrum
 *
 * Test case for the ThreeGppSpectrumPropagationLossModelTest class.
 * 1) checks if the long term components for the direct and the reverse link
 *    are the same
 * 2) checks if the long term component is updated when changing the beamforming
 *    vectors
 * 3) checks if the long term is updated when changing the channel matrix
 */
class ThreeGppSpectrumPropagationLossModelTest : public TestCase
{
public:
  /**
   * Constructor
   */
  ThreeGppSpectrumPropagationLossModelTest ();

  /**
   * Destructor
   */
  virtual ~ThreeGppSpectrumPropagationLossModelTest ();

private:
  /**
   * Build the test scenario
   */
  virtual void DoRun (void);

  /**
   * Points the beam of thisDevice towards otherDevice
   * \param thisDevice the device to configure
   * \param thisAntenna the antenna object associated to thisDevice
   * \param otherDevice the device to communicate with
   * \param otherAntenna the antenna object associated to otherDevice
   */
  void DoBeamforming (Ptr<NetDevice> thisDevice, Ptr<AntennaArrayBasicModel> thisAntenna, Ptr<NetDevice> otherDevice, Ptr<AntennaArrayBasicModel> otherAntenna);

  /**
   * Test of the long term component is correctly updated when the channel
   * matrix is recomputed
   * \param lossModel the ThreeGppSpectrumPropagationLossModel object used to
   *        compute the rx PSD
   * \param txPsd the PSD of the transmitted signal
   * \param txMob the mobility model of the tx device
   * \param rxMob the mobility model of the rx device
   * \param rxPsdOld the previously received PSD
   */
  void CheckLongTermUpdate (Ptr<ThreeGppSpectrumPropagationLossModel> lossModel, Ptr<SpectrumValue> txPsd, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<SpectrumValue> rxPsdOld);

  /**
   * Checks if two PSDs are equal
   * \param first the first PSD
   * \param second the second PSD
   * \return true if first and second are equal, false otherwise
   */
  static bool ArePsdEqual (Ptr<SpectrumValue> first, Ptr<SpectrumValue> second);
};

ThreeGppSpectrumPropagationLossModelTest::ThreeGppSpectrumPropagationLossModelTest ()
  : TestCase ("Test case for the ThreeGppChannel class")
{
}

ThreeGppSpectrumPropagationLossModelTest::~ThreeGppSpectrumPropagationLossModelTest ()
{
}

void
ThreeGppSpectrumPropagationLossModelTest::DoBeamforming (Ptr<NetDevice> thisDevice, Ptr<AntennaArrayBasicModel> thisAntenna, Ptr<NetDevice> otherDevice, Ptr<AntennaArrayBasicModel> otherAntenna)
{
  uint8_t noPlane = 1;
  complexVector_t antennaWeights;

  Vector aPos = thisDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  Vector bPos = otherDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();

  Angles completeAngle (bPos,aPos);

  double posX = bPos.x - aPos.x;
  double phiAngle = atan ((bPos.y - aPos.y) / posX);

  if (posX < 0)
    {
      phiAngle = phiAngle + M_PI;
    }
  if (phiAngle < 0)
    {
      phiAngle = phiAngle + 2 * M_PI;
    }

  double hAngleRadian = fmod ((phiAngle + (M_PI / noPlane)),2 * M_PI / noPlane) - (M_PI / noPlane);
  double vAngleRadian = completeAngle.theta;

  uint16_t antennaNum [2];
  antennaNum[0] = thisAntenna->GetAntennaNumDim1 ();
  antennaNum[1] = thisAntenna->GetAntennaNumDim2 ();
  int totNoArrayElements = antennaNum[0]*antennaNum[1];
  double power = 1 / sqrt (totNoArrayElements);

  for (int ind = 0; ind < totNoArrayElements; ind++)
    {
      Vector loc = thisAntenna->GetAntennaLocation (ind);
      double phase = -2 * M_PI * (sin (vAngleRadian) * cos (hAngleRadian) * loc.x
                                  + sin (vAngleRadian) * sin (hAngleRadian) * loc.y
                                  + cos (vAngleRadian) * loc.z);
      antennaWeights.push_back (exp (std::complex<double> (0, phase)) * power);
    }

  AntennaArrayBasicModel::BeamId bid = 0;
  thisAntenna->SetBeamformingVector (antennaWeights, bid, otherDevice);
}

bool
ThreeGppSpectrumPropagationLossModelTest::ArePsdEqual (Ptr<SpectrumValue> first, Ptr<SpectrumValue> second)
{
  bool ret = true;
  for (uint8_t i = 0; i < first->GetSpectrumModel ()->GetNumBands (); i++)
  {
    if ((*first) [i] != (*second) [i])
    {
      ret = false;
      continue;
    }
  }
  return ret;
}

void
ThreeGppSpectrumPropagationLossModelTest::CheckLongTermUpdate (Ptr<ThreeGppSpectrumPropagationLossModel> lossModel, Ptr<SpectrumValue> txPsd, Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob, Ptr<SpectrumValue> rxPsdOld)
{
  Ptr<SpectrumValue> rxPsdNew = lossModel->DoCalcRxPowerSpectralDensity (txPsd, txMob, rxMob);
  NS_TEST_ASSERT_MSG_EQ (ArePsdEqual (rxPsdOld, rxPsdNew),  false, "The long term is not updated when the channel matrix is recomputed");
}

void
ThreeGppSpectrumPropagationLossModelTest::DoRun ()
{
  // Build the scenario for the test
  Config::SetDefault ("ns3::ThreeGppChannel::UpdatePeriod", TimeValue (MilliSeconds (100)));

  uint8_t txAntennaElements[] {4, 4}; // tx antenna dimensions
  uint8_t rxAntennaElements[] {4, 4}; // rx antenna dimensions

  // create the ChannelConditionModel object to be used to retrieve the
  // channel condition
  Ptr<ChannelConditionModel> condModel = CreateObject<ThreeGppUmaChannelConditionModel> ();

  // create the ThreeGppSpectrumPropagationLossModel object, set frequency,
  // scenario and channel condition model to be used
  Ptr<ThreeGppSpectrumPropagationLossModel> lossModel = CreateObject<ThreeGppSpectrumPropagationLossModel> ();
  lossModel->SetFrequency (2.4e9);
  lossModel->SetScenario ("UMa");
  lossModel->SetChannelConditionModel (condModel);  // create the ThreeGppChannel object used to generate the channel matrix
  Ptr<ThreeGppChannel> channelModel = CreateObject<ThreeGppChannel> ();
  channelModel->SetAttribute ("Frequency", DoubleValue (2.4e9));
  channelModel->SetAttribute ("Scenario", StringValue ("UMa"));

  // create the tx and rx nodes
  NodeContainer nodes;
  nodes.Create (2);

  // create the tx and rx devices
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();

  // associate the nodes and the devices
  nodes.Get (0)->AddDevice (txDev);
  txDev->SetNode (nodes.Get (0));
  nodes.Get (1)->AddDevice (rxDev);
  rxDev->SetNode (nodes.Get (1));

  // create the tx and rx mobility models and set their positions
  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel> ();
  txMob->SetPosition (Vector (0.0,0.0,10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel> ();
  rxMob->SetPosition (Vector (15.0,0.0,10.0)); // in this position the channel condition is always LOS

  // associate the nodes and the mobility models
  nodes.Get (0)->AggregateObject (txMob);
  nodes.Get (1)->AggregateObject (rxMob);

  // create the tx and rx antennas and set the their dimensions
  Ptr<AntennaArrayModel> txAntenna = CreateObject<AntennaArrayModel> ();
  txAntenna->SetAntennaNumDim1 (txAntennaElements [0]);
  txAntenna->SetAntennaNumDim2 (txAntennaElements [1]);

  Ptr<AntennaArrayModel> rxAntenna = CreateObject<AntennaArrayModel> ();
  rxAntenna->SetAntennaNumDim1 (rxAntennaElements [0]);
  rxAntenna->SetAntennaNumDim2 (rxAntennaElements [1]);

  // initialize ThreeGppSpectrumPropagationLossModel
  lossModel->AddDevice (txDev, txAntenna);
  lossModel->AddDevice (rxDev, rxAntenna);

  // set the beamforming vectors
  DoBeamforming (txDev, txAntenna, rxDev, rxAntenna);
  DoBeamforming (rxDev, rxAntenna, txDev, txAntenna);

  // create the tx psd
  WifiSpectrumValue5MhzFactory sf;
  double txPower = 0.1; // Watts
  uint32_t channelNumber = 1;
  Ptr<SpectrumValue> txPsd =  sf.CreateTxPowerSpectralDensity (txPower, channelNumber);

  // compute the rx psd
  Ptr<SpectrumValue> rxPsdOld = lossModel->DoCalcRxPowerSpectralDensity (txPsd, txMob, rxMob);

  // 1) check that the rx PSD is equal for both the direct and the reverse channel
  Ptr<SpectrumValue> rxPsdNew = lossModel->DoCalcRxPowerSpectralDensity (txPsd, rxMob, txMob);
  NS_TEST_ASSERT_MSG_EQ (ArePsdEqual (rxPsdOld, rxPsdNew),  true, "The long term for the direct and the reverse channel are different");

  // 2) check if the long term is updated when changing the BF vector
  // change the position of the rx device and recompute the beamforming vectors
  rxMob->SetPosition (Vector (10.0, 5.0, 10.0));
  AntennaArrayBasicModel::BeamformingVector txBfVector = txAntenna->GetCurrentBeamformingVector ();
  txBfVector.first [0] = std::complex<double> (0.0, 0.0);
  txAntenna->SetBeamformingVector (txBfVector.first, txBfVector.second, rxDev);

  rxPsdNew = lossModel->DoCalcRxPowerSpectralDensity (txPsd, rxMob, txMob);
  NS_TEST_ASSERT_MSG_EQ (ArePsdEqual (rxPsdOld, rxPsdNew),  false, "Changing the BF vectors the rx PSD does not change");

  // update rxPsdOld
  rxPsdOld = rxPsdNew;

  // 3) check if the long term is updated when the channel matrix is recomputed
  Simulator::Schedule (MilliSeconds (101), &ThreeGppSpectrumPropagationLossModelTest::CheckLongTermUpdate, this, lossModel, txPsd, txMob, rxMob, rxPsdOld);

  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup spectrum
 *
 * Test suite for the ThreeGppChannel class
 */
class ThreeGppChannelTestSuite : public TestSuite
{
public:
  /**
   * Constructor
   */
  ThreeGppChannelTestSuite ();
};

ThreeGppChannelTestSuite::ThreeGppChannelTestSuite ()
  : TestSuite ("three-gpp-channel", UNIT)
{
  AddTestCase (new ThreeGppChannelTest, TestCase::QUICK);
  AddTestCase (new ThreeGppSpectrumPropagationLossModelTest, TestCase::QUICK);
}

static ThreeGppChannelTestSuite myTestSuite;
