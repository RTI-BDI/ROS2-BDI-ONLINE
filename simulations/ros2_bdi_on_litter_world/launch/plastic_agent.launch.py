import sys
import os
import os.path

from ament_index_python.packages import get_package_share_directory

ros2_bdi_bringup_dir = get_package_share_directory('ros2_bdi_bringup')
sys.path.append(ros2_bdi_bringup_dir + '/launch/')
from bdi_agent import AgentLaunchDescription
from bdi_agent_skills import AgentAction
from bdi_agent_skills import AgentSensor


def generate_launch_description():
    PLASTIC_AGENT_ID = 'plastic_agent'
    RECYCLER_AGENTS_GROUP_ID = 'recyclers'

    bdi_on_litterworld_share_dir = get_package_share_directory('ros2_bdi_on_litter_world')

    # plastic_agent move action
    plastic_agent_move = AgentAction(
        package='ros2_bdi_on_litter_world',
        executable='move',
        name=PLASTIC_AGENT_ID+'_move',
        specific_params=[]
    )
    
    # plastic_agent pickup action
    plastic_agent_pickup = AgentAction(
        package='ros2_bdi_on_litter_world',
        executable='litter_pickup',
        name=PLASTIC_AGENT_ID+'litter_pickup',
        specific_params=[]
    )

    # plastic_agent recycle action
    plastic_agent_recycle = AgentAction(
        package='ros2_bdi_on_litter_world',
        executable='recycle_plastic',
        name=PLASTIC_AGENT_ID+'recycle_plastic',
        specific_params=[]
    )

    # get static map info status sensor 
    load_map_sensor = AgentSensor(
        package='ros2_bdi_on_litter_world',
        executable='load_map_sensor',
        name=PLASTIC_AGENT_ID+'_load_map',
        specific_params=[
            {"init_sleep": 2},
            {"sensing_freq": 1.0}
        ])

    # get dynamic map info in a restricted grid 
    agent_area_sensor = AgentSensor(
        package='ros2_bdi_on_litter_world',
        executable='agent_area_sensor',
        name=PLASTIC_AGENT_ID+'_area_sensor',
        specific_params=[
            {"init_sleep": 4},
            {"sensing_freq": 1.0},
            {"detection_depth": 2},
            {"should_patrol": True}
        ])

    pmode = 'online'
    reschedule_policy = 'NO_PREEMPT'
    if pmode == 'online':
        reschedule_policy = 'CLEAN_PREEMPT'

    plastic_agent_ld = AgentLaunchDescription(
        agent_id=PLASTIC_AGENT_ID,
        agent_group=RECYCLER_AGENTS_GROUP_ID,
        init_params={
            'pddl_file': os.path.join(bdi_on_litterworld_share_dir, 'pddl', 'recycling-agent-domain.pddl'),
            'init_bset': os.path.join(bdi_on_litterworld_share_dir, 'launch', 'plastic_agent_init', 'init_bset.yaml'),
            'init_reactive_rules_set': os.path.join(bdi_on_litterworld_share_dir, 'launch', 'plastic_agent_init', 'init_rrules.yaml'.format(pmode)),
            'planner': 'JAVAFF',
            'comp_plan_tries': 4,
            'exec_plan_tries': 4,
            'planning_mode':pmode,
            'reschedule_policy': reschedule_policy,
            'search_interval': 100,
            # 'max_pplan_size': 32,
            'min_commit_steps': 1,
            #'sim_to_n': 3,
            'max_null_search_intervals': 16,
            'debug_log_active': ['javaff', 'scheduler', 'event_listener']
        },
        actions=[plastic_agent_move, plastic_agent_pickup, plastic_agent_recycle],
        sensors=[load_map_sensor, agent_area_sensor],
        run_only_psys2=False
    ) 

    return plastic_agent_ld