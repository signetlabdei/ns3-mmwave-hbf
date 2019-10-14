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

#include "ns3/buildings-channel-condition-model.h"
#include "ns3/mobility-model.h"
#include "ns3/mobility-building-info.h"
#include "ns3/building-list.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BuildingsChannelConditionModel");

NS_OBJECT_ENSURE_REGISTERED (BuildingsChannelConditionModel);

TypeId
BuildingsChannelConditionModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BuildingsChannelConditionModel")
    .SetParent<ChannelConditionModel> ()
    .SetGroupName ("Buildings")
    .AddConstructor<BuildingsChannelConditionModel> ()
  ;
  return tid;
}

BuildingsChannelConditionModel::BuildingsChannelConditionModel ()
  : ChannelConditionModel ()
{
}

BuildingsChannelConditionModel::~BuildingsChannelConditionModel ()
{
}

Ptr<ChannelCondition>
BuildingsChannelConditionModel::GetChannelCondition (Ptr<const MobilityModel> a,
                                                     Ptr<const MobilityModel> b)
{
  Ptr<MobilityBuildingInfo> a1 = a->GetObject<MobilityBuildingInfo> ();
  Ptr<MobilityBuildingInfo> b1 = b->GetObject<MobilityBuildingInfo> ();
  NS_ASSERT_MSG ((a1 != 0) && (b1 != 0), "BuildingsChannelConditionModel only works with MobilityBuildingInfo");

  Ptr<ChannelCondition> cond = CreateObject<ChannelCondition> ();

  // NOTE The IsOutdoor and IsIndoor function is only based on the initial node
  // position, it is not updated when the node is entering the building from
  // outside or vise versa.
  // See https://www.nsnam.org/bugzilla/show_bug.cgi?id=3018
  if (a1->IsOutdoor () && b1->IsOutdoor ())
    {
      // The outdoor case, determine LOS/NLOS
      // The channel condition should be NLOS if the line intersect one of the buildings, otherwise LOS
      bool intersect = IsWithinLineOfSight (a->GetPosition (), b->GetPosition ());
      if (!intersect)
        {
          cond->SetLosCondition (ChannelCondition::LosConditionValue::LOS);
        }
      else
        {
          cond->SetLosCondition (ChannelCondition::LosConditionValue::NLOS);
        }

    }
  else if (a1->IsIndoor () && b1->IsIndoor ())
    {
      // Indoor case, determine is the two nodes are inside the same building
      // or not
      if (a1->GetBuilding () == b1->GetBuilding ())
        {
          cond->SetLosCondition (ChannelCondition::LosConditionValue::LOS);
        }
      else
        {
          cond->SetLosCondition (ChannelCondition::LosConditionValue::NLOS);
        }
    }
  else //outdoor to indoor case
    {
      cond->SetLosCondition (ChannelCondition::LosConditionValue::NLOS);
    }

  return cond;
}

bool
BuildingsChannelConditionModel::IsWithinLineOfSight (Vector L1, Vector L2 ) const
{
  for (BuildingList::Iterator bit = BuildingList::Begin (); bit != BuildingList::End (); ++bit)
    {
      Box boundaries = (*bit)->GetBoundaries ();

      Vector boxSize (0.5 * (boundaries.xMax - boundaries.xMin),
                      0.5 * (boundaries.yMax - boundaries.yMin),
                      0.5 * (boundaries.zMax - boundaries.zMin));
      Vector boxCenter (boundaries.xMin + boxSize.x,
                        boundaries.yMin + boxSize.y,
                        boundaries.zMin + boxSize.z);

      // Put line in box space
      Vector LB1 (L1.x - boxCenter.x, L1.y - boxCenter.y, L1.z - boxCenter.z);
      Vector LB2 (L2.x - boxCenter.x, L2.y - boxCenter.y, L2.z - boxCenter.z);

      // Get line midpoint and extent
      Vector LMid (0.5 * (LB1.x + LB2.x), 0.5 * (LB1.y + LB2.y), 0.5 * (LB1.z + LB2.z));
      Vector L (LB1.x - LMid.x, LB1.y - LMid.y, LB1.z - LMid.z);
      Vector LExt ( abs (L.x), abs (L.y), abs (L.z) );

      // Use Separating Axis Test
      // Separation vector from box center to line center is LMid, since the line is in box space
      // If the line did not intersect this building, jump to the next building.
      if ( abs ( LMid.x ) > boxSize.x + LExt.x )
        {
          continue;
        }
      if ( abs ( LMid.y ) > boxSize.y + LExt.y )
        {
          continue;
        }
      if ( abs ( LMid.z ) > boxSize.z + LExt.z )
        {
          continue;
        }
      // Crossproducts of line and each axis
      if ( abs ( LMid.y * L.z - LMid.z * L.y)  >  (boxSize.y * LExt.z + boxSize.z * LExt.y) )
        {
          continue;
        }
      if ( abs ( LMid.x * L.z - LMid.z * L.x)  >  (boxSize.x * LExt.z + boxSize.z * LExt.x) )
        {
          continue;
        }
      if ( abs ( LMid.x * L.y - LMid.y * L.x)  >  (boxSize.x * LExt.y + boxSize.y * LExt.x) )
        {
          continue;
        }

      // No separating axis, the line intersects
      // If the line intersect this building, return true.
      return true;
    }
  return false;
}

int64_t
BuildingsChannelConditionModel::AssignStreams (int64_t stream)
{
  return 0;
}

} // end namespace ns3
