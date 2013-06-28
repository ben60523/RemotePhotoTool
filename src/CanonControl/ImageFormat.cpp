//
// RemotePhotoTool - remote camera control software
// Copyright (C) 2008-2013 Michael Fink
//
//! \file ImageFormat.cpp Canon control - Image format
//

// includes
#include "stdafx.h"
#include "ImageFormat.hpp"
#include "EdsdkPropertyAccess.hpp"

CString ImageFormat::ToString() const
{
   if (m_value.m_enSDKVariant == T_enSDKVariant::variantEdsdk)
      return EDSDK::PropertyAccess::FormatImageFormatValue(m_value.Value().Get<unsigned int>());

   // TODO implement for PSREC
   return _T("???");
}