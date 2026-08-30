#include "MDBusArbiter.h"

unsigned int MDBusArbiter::GetIMegaDriveResetVersion() const
{
	return IMegaDriveReset::ThisIMegaDriveResetVersion();
}

bool MDBusArbiter::IsMegaDriveResetReady() const
{
	// Do not advertise this operation until the coordinator can observe the
	// architectural M68000 vector fetch and prove RAM/VDP preservation.  Bus
	// and line presence alone is not sufficient and must never cause a
	// best-effort reset to become visible as a supported capability.
	return false;
}

bool MDBusArbiter::ExecuteMegaDriveSoftReset(ISystemExtensionInterface&, SoftResetResult& result)
{
	// The interface is installed before the complete architectural fetch
	// observer is available.  Never pulse a line and then claim success: the
	// system layer and Go bridge treat this as an unavailable native feature.
	result = SoftResetResult();
	result.status = SoftResetResult::Unavailable;
	result.interfaceVersion = ISystemResetInterface::ThisISystemResetInterfaceVersion();
	result.implementationVersion = 1;
	result.failureCode = "soft_reset_unavailable";
	result.failureDetail = "M68000 architectural reset-vector fetch observation is not available in this fork build.";
	return false;
}
