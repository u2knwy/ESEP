/*
 * fsm.cpp
 *
 *  Created on: 24.05.2023
 *      Author: Maik
 */

/*
 * hal.cpp
 *
 *  Created on: 04.04.2023
 *      Author: Maik
 */
#include "mocks/EventManagerMock.h"
#include "mocks/EventSenderMock.h"

#include "logic/main_fsm/MainContext.h"

#include <gtest/gtest.h>

class UnitTest_MainFSM : public ::testing::Test {
  protected:
	std::shared_ptr<EventManagerMock> evm;
    IEventSender* sender;
    MainActions* mainActions;
    MainContext* fsm;

    void SetUp() override {
    	evm = std::make_shared<EventManagerMock>();
    	sender = new EventSenderMock();
        mainActions = new MainActions(evm, sender);
        fsm = new MainContext(mainActions);
    }

    void TearDown() override {
    	delete mainActions;
    	delete fsm;
    }
};

// When launching the FSM -> Current state must be STANDBY
TEST_F(UnitTest_MainFSM, StartStateStandby) {
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
    EXPECT_TRUE(true);
}

// Start pressed short -> Current state must be RUNNING
TEST_F(UnitTest_MainFSM, StateRunningAfterStartPressedShort) {
    fsm->master_btnStart_PressedShort();
    EXPECT_EQ(MainState::RUNNING, fsm->getCurrentState());

    delete fsm;
	sender = new EventSenderMock();
    mainActions = new MainActions(evm, sender);
    fsm = new MainContext(mainActions);

    fsm->slave_btnStart_PressedShort();
    EXPECT_EQ(MainState::RUNNING, fsm->getCurrentState());
}

// Start pressed long -> Current state must be SERVICE_MODE
TEST_F(UnitTest_MainFSM, StateServiceModeAfterStartPressedLong) {
    fsm->master_btnStart_PressedLong();
    EXPECT_EQ(MainState::SERVICEMODE,
              fsm->getCurrentState());                       // in ServiceMode
    fsm->master_btnStop_Pressed();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());   // back in Standby

    delete fsm;
	sender = new EventSenderMock();
    mainActions = new MainActions(evm, sender);
    fsm = new MainContext(mainActions);

    fsm->slave_btnStart_PressedLong();
    EXPECT_EQ(MainState::SERVICEMODE, fsm->getCurrentState()); // in ServiceMode

    // Calibrate offset
    evm->clearLastHandledEvents();
    fsm->master_btnStart_PressedShort();
    EXPECT_TRUE(evm->lastHandledEventsContain(Event{HM_M_CAL_OFFSET}));
    fsm->master_btnReset_Pressed();

    // Calibrate reference
    evm->clearLastHandledEvents();
    fsm->master_btnStart_PressedShort();
    EXPECT_TRUE(evm->lastHandledEventsContain(Event{HM_M_CAL_REF}));
    fsm->master_btnReset_Pressed();

    // Selftest Sensors
    evm->clearLastHandledEvents();
    fsm->master_LBA_Blocked();
    EXPECT_TRUE(evm->lastHandledEventsContain(Event{MOTOR_M_RIGHT_REQ, 1}));
    fsm->master_LBW_Blocked();
    fsm->master_LBE_Blocked();
    fsm->slave_LBA_Blocked();
    EXPECT_TRUE(evm->lastHandledEventsContain(Event{MOTOR_S_RIGHT_REQ, 1}));
    fsm->slave_LBW_Blocked();
    fsm->slave_LBE_Blocked();
    fsm->master_LBR_Blocked();
    fsm->slave_LBR_Blocked();
    evm->clearLastHandledEvents();
    fsm->master_btnStart_PressedShort();
    // All tests pass - still in ServiceMode in SelftestActuators
    EXPECT_EQ(MainState::SERVICEMODE, fsm->getCurrentState());

    // Selftest actuators
    evm->lastHandledEventsContain(Event{EventType::LAMP_M_GREEN, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LAMP_M_YELLOW, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LAMP_M_RED, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_M_START, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_M_RESET, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_M_Q1, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_M_Q2, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LAMP_S_GREEN, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LAMP_S_YELLOW, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LAMP_S_RED, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_S_START, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_S_RESET, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_S_Q1, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::LED_S_Q2, (int) LampState::ON});
    evm->lastHandledEventsContain(Event{EventType::SORT_M_OUT, 0});
    evm->lastHandledEventsContain(Event{EventType::SORT_S_OUT, 0});

    // Start -> actuators also pass - go to Standby
    fsm->master_btnStart_PressedShort();
    evm->clearLastHandledEvents();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
    evm->lastHandledEventsContain(Event{EventType::MODE_STANDBY});
}

// In state running and stop pressed -> Current state must be STANDBY
TEST_F(UnitTest_MainFSM, StateStandbyAfterRunningAndStopPressed) {
    // Buttons pressed at master
    fsm->master_btnStart_PressedShort();
    fsm->master_btnStop_Pressed();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());

    // Buttons pressed at slave
    fsm->slave_btnStart_PressedShort();
    fsm->slave_btnStop_Pressed();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
}

// In state running and EStop pressed -> Current state must be ESTOP
TEST_F(UnitTest_MainFSM, StateEStopAfterRunningAndEStopPressed) {
    // Buttons pressed at master
    fsm->master_btnStart_PressedShort();
    EXPECT_EQ(MainState::RUNNING, fsm->getCurrentState());
    fsm->master_EStop_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());   // must be in EStop

    // One EStop pressed -> must still stay in EStop
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());
    fsm->slave_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());

    // Second EStop pressed -> must still stay in EStop
    fsm->slave_EStop_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());
    fsm->slave_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());

    // First EStop released -> must still stay in EStop
    fsm->master_EStop_Released();
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());
    fsm->slave_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());

    // Second EStop released -> must go to Standby if both reset buttons pressed
    fsm->slave_EStop_Released();
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::ESTOP, fsm->getCurrentState());     // still in EStop
    fsm->slave_btnReset_Pressed();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());   // now in Standby
}

TEST_F(UnitTest_MainFSM, ErrorManualSolvedAfterRunning) {
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
    fsm->master_btnStart_PressedShort();
    EXPECT_EQ(MainState::RUNNING, fsm->getCurrentState());

    fsm->nonSelfSolvableErrorOccurred();
    EXPECT_EQ(MainState::ERROR, fsm->getCurrentState());
    fsm->master_btnStart_PressedShort();
    EXPECT_EQ(MainState::ERROR, fsm->getCurrentState());
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::ERROR, fsm->getCurrentState());
    fsm->master_btnStart_PressedShort();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
}

TEST_F(UnitTest_MainFSM, ErrorSelfSolvedAfterRunning) {
    // Running -> Error
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
    fsm->master_btnStart_PressedShort();
    EXPECT_EQ(MainState::RUNNING, fsm->getCurrentState());

    // Self-solvable error -> Error Pending Unresigned
    // selfSolving=true, manSolving=false
    fsm->selfSolvableErrorOccurred();
    EXPECT_EQ(MainState::ERROR, fsm->getCurrentState());
    fsm->master_btnStart_PressedShort();
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::ERROR, fsm->getCurrentState());

    // Error was solved -> Solved unresigned
    // selfSolving=false, manSolving=false
    fsm->errorSelfSolved();
    EXPECT_EQ(MainState::ERROR, fsm->getCurrentState());
    // Reset pressed -> Leave Error mode
    fsm->master_btnReset_Pressed();
    EXPECT_EQ(MainState::STANDBY, fsm->getCurrentState());
}
