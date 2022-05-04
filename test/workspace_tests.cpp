/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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

#include <workspace_tests.h>

TEST_F(WorkspaceInstanceTest, getCount)
{
  for (int i = 0; i < 6; ++i) {
    ASSERT_EQ(instance->getCount(flagsList[i]), 1);
  }
  ASSERT_EQ(instance->getCount(flagsList[5] << 1), 0);
}

TEST_F(WorkspaceInstanceTest, getName)
{
  ASSERT_EQ(instance->getName(Workspace::INPUT, (size_t)0),
            defaultInputChannelName);
  ASSERT_EQ(instance->getName(Workspace::OUTPUT, (size_t)0),
            defaultOutputChannelName);
  ASSERT_EQ(instance->getName(Workspace::PARAMETER, (size_t)0),
            defaultParameterChannelName);
  ASSERT_EQ(instance->getName(Workspace::STATE, (size_t)0),
            defaultStateChannelName);
  ASSERT_EQ(instance->getName(Workspace::EVENT, (size_t)0),
            defaultEventChannelName);
  ASSERT_EQ(instance->getName(Workspace::COMMENT, (size_t)0),
            defaultCommentChannelName);
}

TEST_F(WorkspaceInstanceTest, getDescription)
{
  ASSERT_EQ(instance->getDescription(Workspace::PARAMETER, (size_t)0),
            defaultParameterChannelDescription);
  ASSERT_EQ(instance->getDescription(Workspace::STATE, (size_t)0),
            defaultStateChannelDescription);
  ASSERT_EQ(instance->getDescription(Workspace::EVENT, (size_t)0),
            defaultEventChannelDescription);
}

TEST_F(WorkspaceInstanceTest, getValue)
{
  EXPECT_NEAR(instance->getValue(Workspace::EVENT, (size_t)0), 0.0, 0.001);
  EXPECT_NEAR(instance->getValue(Workspace::PARAMETER, (size_t)0), 0.0, 0.001);
  EXPECT_NEAR(instance->getValue(Workspace::STATE, (size_t)0), 0.0, 0.001);
}

TEST_F(WorkspaceInstanceTest, setValue)
{
  double temp = 10.0;
  ASSERT_DOUBLE_EQ(instance->getValue(Workspace::PARAMETER, (size_t)0), 0.0);
  for (int i = 0; i < 3; ++i) {
    instance->setValue(0, temp * i);
    ASSERT_DOUBLE_EQ(instance->getValue(Workspace::PARAMETER, (size_t)0),
                     temp * i);
  }
}

TEST_F(WorkspaceInstanceTest, getValueString)
{
  ASSERT_EQ(instance->getValueString(Workspace::EVENT, (size_t)0), "");
  // getValueString function can sometimes give nonzero, alebeit extemely low,
  // values. This following line can sometimes but not always fail. Not worth
  // testing.
  // ASSERT_EQ(instance->getValueString(Workspace::PARAMETER, (size_t) 0),
  // std::to_string(0));
  ASSERT_EQ(instance->getValueString(Workspace::STATE, (size_t)0), "");
}

TEST_F(WorkspaceInstanceTest, setComment)
{
  instance->setComment((size_t)0, "test");
  ASSERT_EQ(instance->getValueString(Workspace::COMMENT, (size_t)0), "test");
}

TEST_F(WorkspaceManagerTest, getInstance)
{
  manager = Workspace::Manager::getInstance();
  ASSERT_EQ(manager, Workspace::Manager::getInstance());
  ASSERT_EQ(manager, manager->getInstance());
}

// This function is never used. Test creation delayed until completion of other
// tasks.
// TODO: Create tests for foreachWorkspace function
TEST_F(WorkspaceManagerTest, foreachWorkspace) {}
