/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 University of Padova, Dep. of Information Engineering, SIGNET lab.
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
*/

#ifndef SRC_MMWAVE_BEAMFORMING_MODEL_H_
#define SRC_MMWAVE_BEAMFORMING_MODEL_H_

#include "ns3/object.h"
#include "ns3/antenna-array-basic-model.h"
#include "ns3/mobility-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/spectrum-propagation-loss-model.h"
#include "ns3/mmwave-phy-mac-common.h"
#include <map>
#include <valarray>

namespace ns3 {

class MobilityModel;
class AntennaArrayBasicModel;
class NetDevice;

namespace mmwave {


typedef std::vector< std::complex<double> > complexVector_t; //!< type definition for complex vectors
typedef std::vector<complexVector_t> complex2DVector_t; //!< type definition for complex matrices

/**
 * This class handles the beamforming operations.
 * Extend this class to implement a specific beamforming algorithm.
 */
class MmWaveBeamformingModel : public Object
{
public:
  /**
   * Constructor
   */
  MmWaveBeamformingModel ();

  /**
   * Destructor
   */
  virtual ~MmWaveBeamformingModel ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
    * Returns the antenna object
    * \return the antenna object
    */
   Ptr<AntennaArrayBasicModel> GetAntenna (void) const;

   /**
      * Set the antenna array object.
      * \param antenna the antenna object
      */
     void SetAntenna (Ptr<AntennaArrayBasicModel> antenna);

     /**
        * Set the mobility model object.
        * \param mobility model of the device
        */
     void SetMobilityModel (Ptr<MobilityModel> mm);


     /**
        * Sets the spectrum propagation loss model that may be used to read channel values
        * (WARNING: reading the channel directly from the model rather than from device status implies assuming perfect CSI for beamforming)
        * \param spectrumPropagationLossModel the spectrum propagation loss model
        */
     void SetSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> spectrumPropagationLossModel);

     /**
      * Sets the propagation loss model that may be used to read channel values
      * (WARNING: reading the channel directly from the model rather than from device status implies assuming perfect CSI for beamforming)
      * \param propagationLossModel the spectrum propagation loss model
      */
     void SetPropagationLossModel (Ptr<PropagationLossModel> propagationLossModel);

     /**
      * Sets the mmwave phy mac parameters, allowing to use SpectrumValueHelper by the beamforming model
      * \param ptrConfig the configuration parameters
      */
     void SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig);

     /**
      * Assigns the beamforming vector to communicate with the target device
      * and sets the antenna.
      * \param the target device
      */
     virtual void SetBeamformingVectorForDevice (Ptr<NetDevice> device, uint8_t layerInd = 0) = 0;

protected:
  Ptr<AntennaArrayBasicModel> m_antenna; // pointer to the antenna array instance
  Ptr<MobilityModel> m_mobility; // pointer to the MobilityModel installed in this device
  Ptr<SpectrumPropagationLossModel> m_spectrumPropagationLossModel; // pointer to the multipath channel impulse response model
  Ptr<PropagationLossModel> m_propagationLossModel; // pointer to the pathloss model
  Ptr<MmWavePhyMacCommon> m_mmWavePhyMacConfig;
};


struct BFVectorCacheEntry : public SimpleRefCount<BFVectorCacheEntry>
{
  Vector m_myPos; //the semantic here is my position and other device position instead of tx and rx because we assume reversible channels
  Vector m_otherPos;
  AntennaArrayBasicModel::BeamId m_beamId;
  AntennaArrayBasicModel::complexVector_t m_antennaWeights;
  virtual ~BFVectorCacheEntry() {} ;//this makes the struct polymorphic, so it can be extended by new beamforming classes that need to store more info in cache
};

/**
 * This class extends the MmWaveBeamformingModel interface.
 * It implements a DFT-based beamforming algorithm.
 */
class MmWaveDftBeamforming : public MmWaveBeamformingModel
{
public:
  /**
   * Constructor
   */
  MmWaveDftBeamforming ();

  /**
   * Destructor
   */
  virtual ~MmWaveDftBeamforming ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * Computes the beamforming vector to communicate with the target device
   * and sets the antenna.
   * The beamforming vector is computed using a DFT-based beamforming
   * algorithm.
   * \param the target device
   */
  virtual void SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice, uint8_t layerInd = 0) override;

  /**
   * Computes the beamforming vector to communicate with the target device
   * using the angle from the position information
   * \param the target device
   */
  virtual AntennaArrayBasicModel::BeamformingVector DoDesignBeamformingVectorForDevice (Ptr<NetDevice> otherDevice);

  /**
     * Checks the expiration of a BF vector cache entry. Overriding this function can let subclasses of this class modify the cache  behavior
     * \param the target device
     * \param the target cache entry
     */
  virtual bool CheckBfCacheExpiration(Ptr<NetDevice> otherDevice, Ptr<BFVectorCacheEntry> pCacheValue);

protected:

   /**
    * Calculate the channel key using the Cantor function
    * \param x1 first value
    * \param x2 second value
    * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$
    */
   static constexpr uint32_t GetKey (uint32_t x1, uint32_t x2)
   {//TODO this is a replica of ThreeGppChannel id generation, it is not required to import modification to that function, but it should be considered
     return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
   }
   std::map< uint32_t, Ptr<BFVectorCacheEntry> > m_vectorCache; // a memory to remember previous bf vectors and reuse them without recomputing
};


typedef std::valarray<std::complex<double>> ComplexArray_t;

struct CodebookBFVectorCacheEntry : public BFVectorCacheEntry
{
  Time m_generatedTime;
  uint16_t txBeamInd ;
  uint16_t rxBeamInd ;
  complex2DVector_t m_equivalentChanCoefs; // remember the equivalent channel for all tested pais of tx-rx bf vectors.
};

/**
 * This class extends the MmWaveBeamformingModel interface.
 * It implements a FFT-codebook beamforming algorithm.
 */
class MmWaveFFTCodebookBeamforming : public MmWaveDftBeamforming
{
public:
  /**
   * Constructor
   */
  MmWaveFFTCodebookBeamforming ();

  /**
   * Destructor
   */
  virtual ~MmWaveFFTCodebookBeamforming ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * Computes the beamforming vector to communicate with the target device
   * and sets the antenna.
   * The beamforming vector is computed using a FFT-codebook
   * \param the target device
   */
//  void SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice, uint8_t layerInd = 0);//we do not need to override this function, only the call to the next one


  /**
   * Computes the beamforming vector to communicate with the target device
   * using using a FFT-codebook and assuming perfect CSI (reads channel matrix from propagation loss model)
   * \param the target device
   */
  virtual AntennaArrayBasicModel::BeamformingVector DoDesignBeamformingVectorForDevice (Ptr<NetDevice> otherDevice) override;

  /**
      * Checks the expiration of a BF vector cache entry. Overriding this function can let subclasses of this class modify the cache  behavior
      * \param the target device
      * \param the target cache entry
      */
   virtual bool CheckBfCacheExpiration(Ptr<NetDevice> otherDevice, Ptr<BFVectorCacheEntry> pCacheValue) override;

   virtual std::pair<uint16_t,uint16_t> bfGainLookup(complex2DVector_t& equivalentChannelCoefs, std::set<uint16_t> blockedTxIdx = {});
   complexVector_t bfVector2DFFT(uint16_t index, uint16_t antennaNum [2]);

private:

  static constexpr double PI = 3.141592653589793238460;
  void InPlaceArrayFFT (ComplexArray_t& x, bool inv = false);
  void Channel4DFFT (complex2DVector_t& matrix,Ptr<NetDevice> otherDevice);
};


struct MMSEBFVectorCacheEntry : public SimpleRefCount<MMSEBFVectorCacheEntry>
{
  Time m_generatedTime;
  //user pair tuple for all txs
  //bf vectors collection
};

/**
 * This class extends the MmWaveFFTCodebookBeamforming interface.
 * It implements a MMSE digital stage based on FFTCodebook analog beams.
 */
class MmWaveMMSEBeamforming : public MmWaveFFTCodebookBeamforming
{
public:
  /**
   * Constructor
   */
  MmWaveMMSEBeamforming ();

  /**
   * Destructor
   */
  virtual ~MmWaveMMSEBeamforming ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
      * Sets the MMSE bf vectors for a bundle of slots
      * \param the target device
      * \param the target cache entry
      */
  virtual void SetBeamformingVectorForSlotBundle( std::vector< Ptr<NetDevice> > vOtherDevs , std::vector<uint16_t> vLayerInds);
  void SetNoisePowerSpectralDensity( double noisePSD );

protected:
  complex2DVector_t MmseCholesky (complex2DVector_t matrixH);
  std::vector< Ptr<CodebookBFVectorCacheEntry>> GetBfCachesInSlotBundle(std::vector< Ptr<NetDevice> > vOtherDevs);

private:

  complexVector_t MmseSolve (complex2DVector_t matrixH, complexVector_t y);
  double m_noisePowerSpectralDensity;
};


/**
 * This class extends the MmWaveFFTCodebookBeamforming interface.
 * It implements a frequency-selective MMSE digital stage with better SINR but more computational complexity.
 */
class MmWaveMMSESpectrumBeamforming : public MmWaveMMSEBeamforming
{
public:
  /**
   * Constructor
   */
  MmWaveMMSESpectrumBeamforming ();

  /**
   * Destructor
   */
  virtual ~MmWaveMMSESpectrumBeamforming ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
      * Sets the MMSE bf vectors for a bundle of slots
      * \param the target device
      * \param the target cache entry
      */
  virtual void SetBeamformingVectorForSlotBundle( std::vector< Ptr<NetDevice> > vOtherDevs , std::vector<uint16_t> vLayerInds) override;
private:

  complex2DVector_t MmseSolveSimplified (complex2DVector_t matrixH);
};

} // namespace mmwave
} // namespace ns3

#endif /* SRC_MMWAVE_BEAMFORMING_MODEL_H_ */
