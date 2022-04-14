/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

	 This program is free software: you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation, either version 3 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <rtxiTests.h>
#include <iostream>

void TestEnvironment::SetUp()
{
    argc = 3;
    char *argv[3];
    argv[0] = const_cast<char *>("TestQApp");
    argv[1] = const_cast<char *>("-platform");
    argv[2] = const_cast<char *>("offscreen");
    app = new QApplication(argc, argv);
    if (app){
        std::cout << "QT Application object initialized." << std::endl;
    }else{
        std::cout << "Unable to initialized QT application." << std::endl;
    } 
}

void TestEnvironment::TearDown()
{
    delete app;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new TestEnvironment());
    return RUN_ALL_TESTS();
}
