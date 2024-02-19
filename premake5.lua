-- premake5.lua
workspace "TheRayTracer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "TheRayTracer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "WalnutExternal.lua"
include "TheRayTracer"