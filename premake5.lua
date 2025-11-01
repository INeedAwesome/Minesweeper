workspace "Minesweeper"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Minesweeper"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Minesweeper"