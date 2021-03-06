/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    laplacianFoam

Description
    Solves a simple Laplace equation, e.g. for thermal diffusion in a solid.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "OFstream.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

double GetRandom(double min, double max);

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


    Info << "\n setting particle positions \n" << endl;

    word positioningType
    (
        setParticlePositionsPelletDict.lookup("positioningType")
    );

    if(positioningType == "pellet")
    {   
        dictionary autoPelletPositionSubDict
        (
            setParticlePositionsPelletDict.subDict("autoPelletPositionSubDict")
        );

        scalar particleDiameter
        (
            readScalar(autoPelletPositionSubDict.lookup("particleDiameter"))
        );

        label particleNumber
        (
            readLabel(autoPelletPositionSubDict.lookup("particleNumber"))
        );

        scalar radius
        (
            readScalar(autoPelletPositionSubDict.lookup("radius"))
        );

        scalar length
        (
            readScalar(autoPelletPositionSubDict.lookup("length"))
        );

        vector origin
        (
            autoPelletPositionSubDict.lookup("origin")
        );

        label trialNum
        (
            readLabel(autoPelletPositionSubDict.lookup("trialNumber"))
        );

        label densityR
        (
            readLabel(autoPelletPositionSubDict.lookup("densityRadius"))
        );

        label densityT
        (
            readLabel(autoPelletPositionSubDict.lookup("densityTheta"))
        );

        label densityL
        (
            readLabel(autoPelletPositionSubDict.lookup("densityLength"))
        );


        // Min and Max of each variable were multiplied by densityR,T,L respectively to increase the resolution, since random function I made can only output integer.
        // by doing so, it can output decimal value.
        scalar thetaMin = 0*densityT;
        scalar thetaMax = 360 * densityT;
        scalar rMin = 0*densityR;
        scalar rMax = radius * densityR;
        scalar lMin = -(length-particleDiameter)*densityL/2;
        scalar lMax = (length-particleDiameter)*densityL/2;

        //dummy list to store particles in cylindrical coordinates x is r, y is theta, z is length

        List<vector> dummyCylindic(trialNum);

        //dummy list to store particles in rectangula coordinates

        List<vector> dummyRectangula(trialNum);

        //List for particles settled in cylindrical coordinates

        List<vector> cylindic(particleNumber);

        //List for particles transformed into rectangular coordinates

        List<vector> rectangula(particleNumber);

        // the first particle is stored in cylindic and rectangula list

        cylindic[0].x() = GetRandom(rMin,rMax)/densityR;
        cylindic[0].y() = GetRandom(thetaMin,thetaMax)/densityT;
        cylindic[0].z() = GetRandom(lMin,lMax)/densityL;

        Info << "r is" << cylindic[0].x() << endl;
        Info << "theta is" << cylindic[0].y() << endl;
        Info << "length is " << cylindic[0].z() << endl;


        // cylindic[0] is transormed into rectangular coordinates
        //inside sin is written in radian

        rectangula[0].x() = cylindic[0].x()*Foam::sin(constant::mathematical::pi*cylindic[0].y()/180);
        rectangula[0].y() = cylindic[0].z();
        rectangula[0].z() = cylindic[0].x()*Foam::cos(constant::mathematical::pi*cylindic[0].y()/180);

        Info << "x is" << rectangula[0].x() << endl;
        Info << "y is" << rectangula[0].y() << endl;
        Info << "z is " << rectangula[0].z() << endl;
        
        label l = 0;
        forAll(dummyCylindic,i)
        {
            if( l< particleNumber-1 )
            {
                dummyCylindic[i].x() = GetRandom(rMin,rMax)/densityR;
                dummyCylindic[i].y() = GetRandom(thetaMin,thetaMax)/densityT;
                dummyCylindic[i].z() = GetRandom(lMin,lMax)/densityL;
                Info << " dummyCylindic[ "<<i<<"].x()" << dummyCylindic[i].x() << endl;
                Info << " dummyCylindic[ "<<i<<"].y()" << dummyCylindic[i].y() << endl;
                Info << " dummyCylindic[ "<<i<<"].z()" << dummyCylindic[i].z() << endl;

                dummyRectangula[i].x() = dummyCylindic[i].x()*Foam::sin(constant::mathematical::pi*dummyCylindic[i].y()/180);
                dummyRectangula[i].y() = dummyCylindic[i].z();
                dummyRectangula[i].z() = dummyCylindic[i].x()*Foam::cos(constant::mathematical::pi*dummyCylindic[i].y()/180);
    
                label counter = 0;
                label t;
                for(t=0; t<=l; t++)
                {
                    scalar S;
                    S = mag(dummyRectangula[i]-rectangula[t]);
                    if(S>particleDiameter)
                    {
                        counter += 1;   
                    }else{
                        counter += 0;
                    }
                }
             
                if(counter == l+1)
                {
                    l = l+1;
                    rectangula[l] = dummyRectangula[i];
                }
            }
        } 
        

        fileName rootName(runTime.path());
        fileName fName("particlePositionsPellet");   
        OFstream os(rootName/fName);
        Info<< "Writing to the output file"<<endl;

        label k;
        for(k=0; k<=l; k++)
        {
            rectangula[k] += origin ;
        }

        for(k=0; k<=l; k++)
        {
            os <<"("<<rectangula[k].x()<<"  "<<rectangula[k].y()<<"  "<<rectangula[k].z()<<")"<<endl;
        }


        
        Info<< "End\n" << endl;
    }
    return 0;
}
double GetRandom(double min, double max)
    {
        return min + rand()*(max-min+1.0)/(1.0+RAND_MAX);
    }

// ************************************************************************* //
