import sys
import os
import os.path

from ament_index_python.packages import get_package_share_directory

ros2_bdi_bringup_dir = get_package_share_directory('ros2_bdi_bringup')
sys.path.append(ros2_bdi_bringup_dir + '/launch/')
from bdi_agent import AgentLaunchDescription
from bdi_agent_skills import AgentAction
from bdi_agent_skills import AgentSensor
from webots_ros2_driver.webots_launcher import WebotsLauncher


def generate_launch_description():
    GANTRY_AGENT_ID = 'gripper_a'
    GANTRY_AGENT_GROUP_ID = 'grippers'

    bdi_onwebots_share_dir = get_package_share_directory('ros2_bdi_on_webots')
    
    # move gripper action
    move_gripper = AgentAction(
        package='ros2_bdi_on_webots',
        executable='gripper_move',
        name='gripper_move'
    )

    # gripper pickup action
    gripper_pickup = AgentAction(
        package='ros2_bdi_on_webots',
        executable='gripper_pickup',
        name='gripper_pickup'
    )

    # gripper putdown action
    gripper_putdown = AgentAction(
        package='ros2_bdi_on_webots',
        executable='gripper_putdown',
        name='gripper_putdown'
    )
    
    # gripper put_on_carrier action
    gripper_put_on_carrier = AgentAction(
        package='ros2_bdi_on_webots',
        executable='gripper_put_on_carrier',
        name='gripper_put_on_carrier'
    )
    
    # req carrier to come action istance 1
    req_carrier_to_come1 = AgentAction(
        package='ros2_bdi_on_webots',
        executable='req_carrier_to_come',
        name='req_carrier_to_come1'
    )

    # req carrier to come action istance 2
    req_carrier_to_come2 = AgentAction(
        package='ros2_bdi_on_webots',
        executable='req_carrier_to_come',
        name='req_carrier_to_come2'
    )

    # req carrier to come action istance 3
    req_carrier_to_come3 = AgentAction(
        package='ros2_bdi_on_webots',
        executable='req_carrier_to_come',
        name='req_carrier_to_come3'
    )

    # get carriers status sensor 
    gripper_get_carriers_status = AgentSensor(
        package='ros2_bdi_on_webots',
        executable='gripper_get_carriers_status',
        name=GANTRY_AGENT_ID+'_get_carriers_status',
        specific_params=[
            {"init_sleep": 2},
            {"sensing_freq": 1.0}
        ])

    gantry_agent_ld = AgentLaunchDescription(
        agent_id=GANTRY_AGENT_ID,
        agent_group=GANTRY_AGENT_GROUP_ID,
        init_params={
            'pddl_file': os.path.join(bdi_onwebots_share_dir, 'pddl', 'gripper', 'gripper-domain.pddl'),
            'init_bset': os.path.join(bdi_onwebots_share_dir, 'launch', 'gripper_a_init', 'init_bset_gripper_a.yaml'),
            'init_dset': os.path.join(bdi_onwebots_share_dir, 'launch', 'gripper_a_init', 'init_dset_gripper_a.yaml'),
            'init_reactive_rules_set': os.path.join(bdi_onwebots_share_dir, 'launch', 'gripper_a_init', 'init_rrules_gripper_a.yaml'),
            'planner': 'JAVAFF',
            'debug_log_active': ['belief_manager']
        },
        actions=[move_gripper, gripper_pickup, gripper_putdown, gripper_put_on_carrier, 
                req_carrier_to_come1, 
                req_carrier_to_come2, 
                req_carrier_to_come3, 
        ],
        sensors=[gripper_get_carriers_status],
        run_only_psys2=False
    ) 

    return gantry_agent_ld