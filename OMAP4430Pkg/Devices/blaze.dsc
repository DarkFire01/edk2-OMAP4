[Defines]
  PLATFORM_NAME                  = OMAP4430Pkg
  PLATFORM_GUID                  = 28f1a3bf-193a-47e3-a7b9-5a435eaab2ee
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = OMAP4430Pkg/OMAP4430Pkg.fdf

!include OMAP4430Pkg/OMAP4430Pkg.dsc

[PcdsFixedAtBuild.common]
  # System Memory (1GB)
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x40000000
  
  # Framebuffer (480x854)
  gOMAP4430PkgTokenSpaceGuid.PcdMipiFrameBufferWidth|480
  gOMAP4430PkgTokenSpaceGuid.PcdMipiFrameBufferHeight|864
  gOMAP4430PkgTokenSpaceGuid.PcdMipiFrameBufferVisibleWidth|480
  gOMAP4430PkgTokenSpaceGuid.PcdMipiFrameBufferVisibleHeight|864
  gOMAP4430PkgTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32
