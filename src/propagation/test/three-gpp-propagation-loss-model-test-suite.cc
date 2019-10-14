/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/test.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/channel-condition-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/buildings-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThreeGppPropagationLossModelsTest");

/**
 * Test case for the class ThreeGppRmaPropagationLossModel.
 * It computes the pathloss between two nodes and compares it with the value
 * obtained using the formula in 3GPP TR 38.901, Table 7.4.1-1.
 */
class ThreeGppRmaPropagationLossModelTestCase : public TestCase
{
public:
  /**
   * Constructor
   */
  ThreeGppRmaPropagationLossModelTestCase ();

  /**
   * Destructor
   */
  virtual ~ThreeGppRmaPropagationLossModelTestCase ();

private:
  /**
   * Build the simulation scenario and run the tests
   */
  virtual void DoRun (void);

  /**
   * Struct containing the parameters for each test
   */
  typedef struct
  {
    double m_distance; //!< distance between UT and BS
    bool m_isLos; //!< if true LOS, if false NLOS
    double m_frequency; //!< carrier frequency
    double m_pt;  //!< transmitted power
    double m_pr;  //!< received power
    double m_tolerance; //!< tolerance
  } TestVector;

  TestVectors<TestVector> m_testVectors; //!< array containing all the test vectors
};

ThreeGppRmaPropagationLossModelTestCase::ThreeGppRmaPropagationLossModelTestCase ()
  : TestCase ("Test for the ThreeGppRmaPropagationLossModel class"), m_testVectors ()
{
}

ThreeGppRmaPropagationLossModelTestCase::~ThreeGppRmaPropagationLossModelTestCase ()
{
}

void
ThreeGppRmaPropagationLossModelTestCase::DoRun (void)
{
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -77.3784;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -87.2965;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -108.5577;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 10000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -140.3896;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -77.3784;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -95.7718;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -133.5223;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -160.5169;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  // Create the nodes for BS and UT
  NodeContainer nodes;
  nodes.Create (2);

  // Create the mobility models
  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (0)->AggregateObject (a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (1)->AggregateObject (b);

  // Create a building which will be used to simulate the NLOS state
  Ptr<Building> building = Create<Building> ();
  building->SetBoundaries (Box (0.5, 0.75, 99.0, 101.0, 0.0, 50.0));
  BuildingsHelper::Install (nodes);

  // Use the building channel condition model, so that we have a deterministic
  // LOS/NLOS channel state
  Ptr<ChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel> ();

  // Create the propagation loss model
  Ptr<ThreeGppRmaPropagationLossModel> lossModel = CreateObject<ThreeGppRmaPropagationLossModel> ();
  lossModel->SetAttribute ("ShadowingEnabled", BooleanValue (false)); // disable the shadow fading
  lossModel->SetChannelConditionModel (condModel);

  for (uint32_t i = 0; i < m_testVectors.GetN (); i++)
    {
      TestVector testVector = m_testVectors.Get (i);

      Vector posBs = Vector (0.0, 0.0, 35.0);
      Vector posUt = Vector (testVector.m_distance, 0.0, 1.5);

      // If NLOS, put the BS and the UT between the building
      if (!testVector.m_isLos)
        {
          posBs.y = +100.0;
          posUt.y = +100.0;
        }

      a->SetPosition (posBs);
      b->SetPosition (posUt);
      BuildingsHelper::MakeMobilityModelConsistent ();

      lossModel->SetAttribute ("Frequency", DoubleValue (testVector.m_frequency));
      NS_TEST_EXPECT_MSG_EQ_TOL (lossModel->CalcRxPower (testVector.m_pt, a, b), testVector.m_pr, testVector.m_tolerance, "Got unexpected rcv power");
    }
}

/**
 * Test case for the class ThreeGppUmaPropagationLossModel.
 * It computes the pathloss between two nodes and compares it with the value
 * obtained using the formula in 3GPP TR 38.901, Table 7.4.1-1.
 */
class ThreeGppUmaPropagationLossModelTestCase : public TestCase
{
public:
  /**
   * Constructor
   */
  ThreeGppUmaPropagationLossModelTestCase ();

  /**
   * Destructor
   */
  virtual ~ThreeGppUmaPropagationLossModelTestCase ();

private:
  /**
   * Build the simulation scenario and run the tests
   */
  virtual void DoRun (void);

  /**
   * Struct containing the parameters for each test
   */
  typedef struct
  {
    double m_distance; //!< distance between UT and BS
    bool m_isLos; //!< if true LOS, if false NLOS
    double m_frequency; //!< carrier frequency
    double m_pt; //!< transmitted power
    double m_pr; //!< received power
    double m_tolerance; //!< tolerance
  } TestVector;

  TestVectors<TestVector> m_testVectors; //!< array containing all the test vectors
};

ThreeGppUmaPropagationLossModelTestCase::ThreeGppUmaPropagationLossModelTestCase ()
  : TestCase ("Test for the ThreeGppUmaPropagationLossModel class"), m_testVectors ()
{
}

ThreeGppUmaPropagationLossModelTestCase::~ThreeGppUmaPropagationLossModelTestCase ()
{
}

void
ThreeGppUmaPropagationLossModelTestCase::DoRun (void)
{
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -72.9380;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -86.2362;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -109.7252;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -137.6794;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -82.5131;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -106.1356;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -144.7641;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -172.0753;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  // Create the nodes for BS and UT
  NodeContainer nodes;
  nodes.Create (2);

  // Create the mobility models
  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (0)->AggregateObject (a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (1)->AggregateObject (b);

  // Create a building which will be used to simulate the NLOS state
  Ptr<Building> building = Create<Building> ();
  building->SetBoundaries (Box (0.5, 0.75, 99.0, 101.0, 0.0, 50.0));
  BuildingsHelper::Install (nodes);

  // Use the building channel condition model, so that we have a deterministic
  // LOS/NLOS channel state
  Ptr<ChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel> ();

  // Create the propagation loss model
  Ptr<ThreeGppUmaPropagationLossModel> lossModel = CreateObject<ThreeGppUmaPropagationLossModel> ();
  lossModel->SetAttribute ("ShadowingEnabled", BooleanValue (false)); // disable the shadow fading
  lossModel->SetChannelConditionModel (condModel);

  for (uint32_t i = 0; i < m_testVectors.GetN (); i++)
    {
      TestVector testVector = m_testVectors.Get (i);

      Vector posBs = Vector (0.0, 0.0, 25.0);
      Vector posUt = Vector (testVector.m_distance, 0.0, 1.5);

      // If NLOS, put the BS and the UT between the building
      if (!testVector.m_isLos)
        {
          posBs.y = +100.0;
          posUt.y = +100.0;
        }

      a->SetPosition (posBs);
      b->SetPosition (posUt);
      BuildingsHelper::MakeMobilityModelConsistent ();

      lossModel->SetAttribute ("Frequency", DoubleValue (testVector.m_frequency));
      NS_TEST_EXPECT_MSG_EQ_TOL (lossModel->CalcRxPower (testVector.m_pt, a, b), testVector.m_pr, testVector.m_tolerance, "Got unexpected rcv power");
    }
}

/**
 * Test case for the class ThreeGppUmiStreetCanyonPropagationLossModel.
 * It computes the pathloss between two nodes and compares it with the value
 * obtained using the formula in 3GPP TR 38.901, Table 7.4.1-1.
 */
class ThreeGppUmiPropagationLossModelTestCase : public TestCase
{
public:
  /**
   * Constructor
   */
  ThreeGppUmiPropagationLossModelTestCase ();

  /**
   * Destructor
   */
  virtual ~ThreeGppUmiPropagationLossModelTestCase ();

private:
  /**
   * Build the simulation scenario and run the tests
   */
  virtual void DoRun (void);

  /**
   * Struct containing the parameters for each test
   */
  typedef struct
  {
    double m_distance; //!< distance between UT and BS
    bool m_isLos; //!< if true LOS, if false NLOS
    double m_frequency; //!< carrier frequency
    double m_pt;  //!< transmitted power
    double m_pr;  //!< received power
    double m_tolerance; //!< tolerance
  } TestVector;

  TestVectors<TestVector> m_testVectors; //!< array containing all the test vectors
};

ThreeGppUmiPropagationLossModelTestCase::ThreeGppUmiPropagationLossModelTestCase ()
  : TestCase ("Test for the ThreeGppUmiPropagationLossModel class"), m_testVectors ()
{
}

ThreeGppUmiPropagationLossModelTestCase::~ThreeGppUmiPropagationLossModelTestCase ()
{
}

void
ThreeGppUmiPropagationLossModelTestCase::DoRun (void)
{
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -69.8591;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -88.4122;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -119.3114;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -147.2696;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -76.7563;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -107.9432;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -143.1886;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -167.8617;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  // Create the nodes for BS and UT
  NodeContainer nodes;
  nodes.Create (2);

  // Create the mobility models
  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (0)->AggregateObject (a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (1)->AggregateObject (b);

  // Create a building which will be used to simulate the NLOS state
  Ptr<Building> building = Create<Building> ();
  building->SetBoundaries (Box (0.5, 0.75, 99.0, 101.0, 0.0, 50.0));
  BuildingsHelper::Install (nodes);

  // Use the building channel condition model, so that we have a deterministic
  // LOS/NLOS channel state
  Ptr<ChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel> ();

  // Create the propagation loss model
  Ptr<ThreeGppUmiStreetCanyonPropagationLossModel> lossModel = CreateObject<ThreeGppUmiStreetCanyonPropagationLossModel> ();
  lossModel->SetAttribute ("ShadowingEnabled", BooleanValue (false)); // disable the shadow fading
  lossModel->SetChannelConditionModel (condModel);

  for (uint32_t i = 0; i < m_testVectors.GetN (); i++)
    {
      TestVector testVector = m_testVectors.Get (i);

      Vector posBs = Vector (0.0, 0.0, 10.0);
      Vector posUt = Vector (testVector.m_distance, 0.0, 1.5);

      // If NLOS, put the BS and the UT between the building
      if (!testVector.m_isLos)
        {
          posBs.y = +100.0;
          posUt.y = +100.0;
        }

      a->SetPosition (posBs);
      b->SetPosition (posUt);
      BuildingsHelper::MakeMobilityModelConsistent ();

      lossModel->SetAttribute ("Frequency", DoubleValue (testVector.m_frequency));
      NS_TEST_EXPECT_MSG_EQ_TOL (lossModel->CalcRxPower (testVector.m_pt, a, b), testVector.m_pr, testVector.m_tolerance, "Got unexpected rcv power");
    }
}

/**
 * Test case for the class ThreeGppIndoorOfficePropagationLossModel.
 * It computes the pathloss between two nodes and compares it with the value
 * obtained using the formula in 3GPP TR 38.901, Table 7.4.1-1.
 */
class ThreeGppIndoorOfficePropagationLossModelTestCase : public TestCase
{
public:
  /**
   * Constructor
   */
  ThreeGppIndoorOfficePropagationLossModelTestCase ();

  /**
   * Destructor
   */
  virtual ~ThreeGppIndoorOfficePropagationLossModelTestCase ();

private:
  /**
   * Build the simulation scenario and run the tests
   */
  virtual void DoRun (void);

  /**
   * Struct containing the parameters for each test
   */
  typedef struct
  {
    double m_distance; //!< distance between UT and BS
    bool m_isLos; //!< if true LOS, if false NLOS
    double m_frequency; // carrier frequency
    double m_pt;  //!< transmitted power
    double m_pr;  //!< received power
    double m_tolerance; //!< tolerance
  } TestVector;

  TestVectors<TestVector> m_testVectors; //!< array containing all the test vectors
};

ThreeGppIndoorOfficePropagationLossModelTestCase::ThreeGppIndoorOfficePropagationLossModelTestCase ()
  : TestCase ("Test for the ThreeGppIndoorOfficePropagationLossModel class"), m_testVectors ()
{
}

ThreeGppIndoorOfficePropagationLossModelTestCase::~ThreeGppIndoorOfficePropagationLossModelTestCase ()
{
}

void
ThreeGppIndoorOfficePropagationLossModelTestCase::DoRun (void)
{
  TestVector testVector;

  testVector.m_distance = 1.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -50.8072;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -63.7630;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 50.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -75.7750;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -80.9802;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 1.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -50.8072;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -73.1894;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 50.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -99.7824;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -111.3062;
  testVector.m_tolerance = 5e-2;
  m_testVectors.Add (testVector);

  // Create the nodes for BS and UT
  NodeContainer nodes;
  nodes.Create (2);

  // Create the mobility models
  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (0)->AggregateObject (a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (1)->AggregateObject (b);

  // Create a building which will be used to simulate the NLOS state
  Ptr<Building> building = Create<Building> ();
  building->SetBoundaries (Box (0.25, 0.75, 99.0, 101.0, 0.0, 50.0));
  BuildingsHelper::Install (nodes);

  // Use the building channel condition model, so that we have a deterministic
  // LOS/NLOS channel state
  Ptr<ChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel> ();

  // Create the propagation loss model
  Ptr<ThreeGppIndoorOfficePropagationLossModel> lossModel = CreateObject<ThreeGppIndoorOfficePropagationLossModel> ();
  lossModel->SetAttribute ("ShadowingEnabled", BooleanValue (false)); // disable the shadow fading
  lossModel->SetChannelConditionModel (condModel);

  for (uint32_t i = 0; i < m_testVectors.GetN (); i++)
    {
      TestVector testVector = m_testVectors.Get (i);

      Vector posBs = Vector (0.0, 0.0, 3.0);
      Vector posUt = Vector (testVector.m_distance, 0.0, 1.5);

      // If NLOS, put the BS and the UT between the building
      if (!testVector.m_isLos)
        {
          posBs.y = +100.0;
          posUt.y = +100.0;
        }

      a->SetPosition (posBs);
      b->SetPosition (posUt);
      BuildingsHelper::MakeMobilityModelConsistent ();

      lossModel->SetAttribute ("Frequency", DoubleValue (testVector.m_frequency));
      NS_TEST_EXPECT_MSG_EQ_TOL (lossModel->CalcRxPower (testVector.m_pt, a, b), testVector.m_pr, testVector.m_tolerance, "Got unexpected rcv power");
    }
}

// Test to check if the shadow fading is correctly updated
class ThreeGppShadowingTestCase : public TestCase
{
public:
  ThreeGppShadowingTestCase ();
  virtual ~ThreeGppShadowingTestCase ();

private:
  virtual void DoRun (void);

  /**
   * Set the desired channel condition, compute the propagation loss, and check
   * if it is correctly updated
   * \param a the first mobility model
   * \param b the second mobility model
   * \param updated true if the shadowing value should be different from the
   *        previous one, false otherwise
   * \param isLos true if the channel condition has to be set to LOS, false
   *        otherwise
   */
  void EvaluateLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b, bool updated, bool isLos);

  Ptr<ThreeGppPropagationLossModel> m_lossModel; //!< the propagation loss model
  double m_currentLoss; //!< used to store the current loss value
};

ThreeGppShadowingTestCase::ThreeGppShadowingTestCase ()
  : TestCase ("Test to check if the shadow fading is correctly updated")
{
}

ThreeGppShadowingTestCase::~ThreeGppShadowingTestCase ()
{
}

void
ThreeGppShadowingTestCase::EvaluateLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b, bool updated, bool isLos)
{
  // set the channel condition
  if (isLos)
    {
      // place the two nodes in LOS
      a->SetPosition (Vector (0.0, 0.0, 3.0));
      b->SetPosition (Vector (10.0, 0.0, 1.0));
    }
  else
    {
      // place the nodes in between the building
      a->SetPosition (Vector (0.0, 100.0, 3.0));
      b->SetPosition (Vector (10.0, 100.0, 1.0));
    }

  double loss = m_lossModel->CalcRxPower (0, a, b);
  NS_LOG_INFO ("m_currentLoss " << m_currentLoss << " new loss " << loss);

  NS_TEST_EXPECT_MSG_EQ ((loss != m_currentLoss), updated, "The shadow fading is not correctly updated");

  // update m_currentLoss
  m_currentLoss = loss;
}

void
ThreeGppShadowingTestCase::DoRun (void)
{
  // Create the nodes for BS and UT
  NodeContainer nodes;
  nodes.Create (2);

  // Create the mobility models
  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (0)->AggregateObject (a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  nodes.Get (1)->AggregateObject (b);

  // Create a building which will be used to simulate the NLOS state
  Ptr<Building> building = Create<Building> ();
  building->SetBoundaries (Box (0.25, 0.75, 99.0, 101.0, 0.0, 50.0));
  BuildingsHelper::Install (nodes);

  // Use the building channel condition model, so that we have a deterministic
  // LOS/NLOS channel state
  Ptr<ChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel> ();

  // Create the propagation loss model
  m_lossModel = CreateObject<ThreeGppIndoorOfficePropagationLossModel> ();
  m_lossModel->SetAttribute ("Frequency", DoubleValue (3.5e9));
  m_lossModel->SetAttribute ("ShadowingEnabled", BooleanValue (true)); // enable the shadow fading
  m_lossModel->SetAttribute ("UpdatePeriod", TimeValue (MilliSeconds (100))); // update the shadow fading every 100 ms
  m_lossModel->SetChannelConditionModel (condModel);

  // initialize m_currentLoss
  a->SetPosition (Vector (0.0, 0.0, 3.0));
  b->SetPosition (Vector (10.0, 0.0, 1.0));
  m_currentLoss = m_lossModel->CalcRxPower (0, a, b);

  // this is scheduled before the update period, the shadowing loss should not be updated
  Simulator::Schedule (MilliSeconds (1), &ThreeGppShadowingTestCase::EvaluateLoss, this, a, b, 0, 1);

  // this is scheduled after the update period, the shadowing loss should be updated
  Simulator::Schedule (MilliSeconds (101), &ThreeGppShadowingTestCase::EvaluateLoss, this, a, b, 1, 1);

  // change the channel condition to NLOS, the shadowing value should be updated
  Simulator::Schedule (MilliSeconds (102), &ThreeGppShadowingTestCase::EvaluateLoss, this, a, b, 1, 0);

  Simulator::Run ();
  Simulator::Destroy ();
}

class ThreeGppPropagationLossModelsTestSuite : public TestSuite
{
public:
  ThreeGppPropagationLossModelsTestSuite ();
};

ThreeGppPropagationLossModelsTestSuite::ThreeGppPropagationLossModelsTestSuite ()
  : TestSuite ("three-gpp-propagation-loss-model", UNIT)
{
  AddTestCase (new ThreeGppRmaPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase (new ThreeGppUmaPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase (new ThreeGppUmiPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase (new ThreeGppIndoorOfficePropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase (new ThreeGppShadowingTestCase, TestCase::QUICK);
}

static ThreeGppPropagationLossModelsTestSuite propagationLossModelsTestSuite;
