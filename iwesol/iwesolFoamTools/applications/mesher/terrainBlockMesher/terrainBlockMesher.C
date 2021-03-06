/*---------------------------------------------------------------------------*\
                               |
  _____        _______ ____    | IWESOL: IWES Open Library
 |_ _\ \      / / ____/ ___|   |
  | | \ \ /\ / /|  _| \___ \   | Copyright: Fraunhofer Institute for Wind
  | |  \ V  V / | |___ ___) |  | Energy and Energy System Technology IWES
 |___|  \_/\_/  |_____|____/   |
                               | http://www.iwes.fraunhofer.de
                               |
-------------------------------------------------------------------------------
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright  held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of IWESOL and it is based on OpenFOAM.

    IWESOL and OpenFOAM are free software: you can redistribute them and/or modify
    them under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IWESOL and OpenFOAM are distributed in the hope that they will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    terrainBlockMesher

Description

Reference

\*---------------------------------------------------------------------------*/

#include "OManager.h"

#include "argList.H"
#include "Time.H"
#include "fvMesh.H"
#include "searchableSurfaces.H"

#include "TerrainManager.H"

using namespace Foam;
using namespace iwesol;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{

#   include "setRootCase.H"
#   include "createTime.H"

    // Read dictionary
    IOdictionary dict
    (
       IOobject
       (
            "terrainBlockMesherDict",
            runTime.system(),
            runTime,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
       )
    );

    // the coordinate system dictionary:
    const dictionary& cooSysDict = dict.subDict("coordinates");

    // the BlockManager dictionary:
    const dictionary& bmDict = dict.subDict("blockManager");

    // Read geometry
    // ~~~~~~~~~~~~~
    autoPtr< searchableSurfaces > stlSurfaces;
    if(dict.found("stl")){
        Info << "Reading stl surface..." << endl;
        const dictionary& geometryDict = dict.subDict("stl");
        stlSurfaces.set(new searchableSurfaces
        (
            IOobject
            (
                "abc",                      // dummy name
                runTime.time().constant(),     // instance
                //mesh.time().findInstance("triSurface", word::null),// instance
                "triSurface",               // local
                runTime.time(),                // registry
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            geometryDict
        ));
        Info << "...done, after " << runTime.cpuTimeIncrement() << " s."<< endl;
    } else {
    	Info << "No entry 'stl' found in dictionary. Choosing empty landscape." << endl;
    }


    // Create coordinate system
    // ~~~~~~~~~~~~~~~~~~~~~~~~
    CoordinateSystem cooSys(cooSysDict);


    // Create output manager
    // ~~~~~~~~~~~~~~~~~~~~~
    blib::OManager om("constant/polyMesh/blockMeshDict",blib::IO::OFILE::TYPE::OPEN_FOAM );


    // Create BlockManager
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Info << "\nRunning TerrainManager" << endl;

    autoPtr< TerrainManager > bm;
    if(stlSurfaces.valid()){
        bm.set(new TerrainManager(bmDict,&cooSys,&(stlSurfaces()[0])));
    } else {
    	bm.set(new TerrainManager(bmDict,&cooSys));
    }
    if(!bm().calc()){
    	Info << "\nError during terrain setup." << endl;
      	throw;
    }
    if(bmDict.found("check")) bm().check();

    // add to output:
    om.addOLink(&(bm()));
    om.collectAll();
    Info << "TerrainManager finished, after " << runTime.cpuTimeIncrement() << " s.\n"<< endl;


	// write output:
    // ~~~~~~~~~~~~~
	om.write();


    Info<< "\nFinished in = "
        << runTime.elapsedCpuTime() << " s." << endl;
    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
