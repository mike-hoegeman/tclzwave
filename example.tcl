#--------------------------------------------------------------------------
#	example.tcl
#	tcl version of Minimal application to test OpenZWave.
#--------------------------------------------------------------------------
puts stderr "ozw package: [package require ozw]"
namespace eval ::ExampleApp {}

set ::ExampleApp::Initialized false

#typedef struct
#{
#	uint32			m_homeId;
#	uint8			m_nodeId;
#	bool			m_polled;
#	list<ValueID>	m_values;
#}NodeInfo;
#

proc ::ExampleApp::AddNodeInfo {homeid nodeid polled values} {
    set key $homeid.$nodeid
    set ::ExampleApp::Nodes($key) [list \
        -homeid $homeid \
        -nodeid $nodeid \
        -polled $polled \
        -values $values \
    ]
    ::ExampleApp::Log " +++ Added Node $::ExampleApp::Nodes($key)"
}
#static list<NodeInfo*> g_nodes;
array set ::ExampleApp::Nodes {}

#static pthread_mutex_t g_criticalSection;
#static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
#static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

#-----------------------------------------------------------------------------
# <GetNodeInfo>
# Return the NodeInfo object associated with this notification
#-----------------------------------------------------------------------------
# we could use tcl objects or dicts here but what we 
# are doing is simple enough to just use tag/value lists in a tcl array 
# for node info mgmt
proc ::ExampleApp::GetNodeInfo {notification} {
    set key [$notification cget -homeid].[$notification cget -nodeid]
    if {[info exists ::ExampleApp::Nodes($key)]} {
        return $::ExampleApp::Nodes($key)
    }
    return ""
}
proc ::ExampleApp::NodeInfoAddValue {notification value} {
    set key [$notification cget -homeid].[$notification cget -nodeid]
    if {[info exists ::ExampleApp::Nodes($key)]} {
        array set a $::ExampleApp::Nodes($key)
        lappend a(-values) $value
        set ::ExampleApp::Nodes($key) [array get a]
        return true
    } else {
        ::ExampleApp::Log "could not find node into w/ key ( $key ) "
        return false
    }
    ::ExampleApp::Log "logic error"
    return false
}
proc ::ExampleApp::NodeInfoRemoveValue {notification value} {
    set key [$notification cget -homeid].[$notification cget -nodeid]
    if {[info exists ::ExampleApp::Nodes($key)]} {
        array set a $::ExampleApp::Nodes($key)
        set idx [lsearch -exact $a(-values) $value]
        if {$idx == -1} {
            ::ExampleApp::Log \
                "could not find value ( $value ) to remove from nodeinfo $key"
            return false
        }
        # delete found value from list
        set a(-values) [lreplace $a(-values) $idx $idx {}]
        # update array entry w/ new data
        set ::ExampleApp::Nodes($key) [array get a]
        return true
    } else {
        ::ExampleApp::Log "could not find node into w/ key ( $key ) "
    }
    ::ExampleApp::Log "logic error"
    return false
}

proc ::ExampleApp::Log {msg} {
    ::ozw::log write LogLevel_Info "ExampleApp: ** $msg"
}
 

#-----------------------------------------------------------------------------
# <OnNotification>
# Callback that is triggered when a value, group or node changes
#-----------------------------------------------------------------------------
proc ::ExampleApp::OnNotification {notification} {


    ## the tcl extension automatically make a critical section 
    ## wrapper/lock around this proc to avoid conflicts with the main thread
    ## it does this w/ a pthread_mutex_(un)lock( &OZW_MainMutex );
    ## like wise the main loop runs each event it processes in a 
    ## pthread_mutex_(un)lock( &OZW_MainMutex ); wrapping

    set type  [$notification cget -typestring]
    ::ExampleApp::Log " **** OnNotification.. ==> $type"
    switch -exact -- $type Type_ValueAdded {
        if {[::ExampleApp::GetNodeInfo $notification] != ""} {
            ::ExampleApp::NodeInfoAddValue $notification \
                [$notification cget -valueid]
        }
    } Type_ValueRemoved {
        if {[::ExampleApp::GetNodeInfo $notification] != ""} {
            ::ExampleApp::NodeInfoRemoveValue $notification \
                [$notification cget -valueid]
        }
    } Type_ValueChanged {
        ## One of the node values has changed
        if {[set n [::ExampleApp::GetNodeInfo $notification]] != ""} {
            ::ExampleApp::Log "Node Value for $n changed"
        }
    } Type_Group {
        ## One of the node's association groups has changed
        if {[set n [::ExampleApp::GetNodeInfo $notification]] != ""} {
            ::ExampleApp::Log "Node assoc. groups for $n changed"
        }
    } Type_NodeAdded {
        ::ExampleApp::AddNodeInfo \
            [$notification cget -homeid] \
            [$notification cget -nodeid] \
            false \
            {}
    }
#
#		case Notification::Type_NodeRemoved:
#		{
#			// Remove the node from our list
#			uint32 const homeId = _notification->GetHomeId();
#			uint8 const nodeId = _notification->GetNodeId();
#			for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
#			{
#				NodeInfo* nodeInfo = *it;
#				if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
#				{
#					g_nodes.erase( it );
#					delete nodeInfo;
#					break;
#				}
#			}
#			break;
#		}
#
#		case Notification::Type_NodeEvent:
#		{
#			// We have received an event from the node, caused by a
#			// basic_set or hail message.
#			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
#			{
#				nodeInfo = nodeInfo;		// placeholder for real action
#			}
#			break;
#		}
#
#		case Notification::Type_PollingDisabled:
#		{
#			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
#			{
#				nodeInfo->m_polled = false;
#			}
#			break;
#		}
#
#		case Notification::Type_PollingEnabled:
#		{
#			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
#			{
#				nodeInfo->m_polled = true;
#			}
#			break;
#		}
#
#		case Notification::Type_DriverReady:
#		{
#			set $ExampleApp::g_homeId [$notification cget -homeid]
#			break;
#		}
#
#		case Notification::Type_DriverFailed:
#		{
#			set ::ExampleApp::InitFailed true
#			set ::ExampleApp::Initialized true 
#			break;
#		}
#
#		case Notification::Type_AwakeNodesQueried:
#		case Notification::Type_AllNodesQueried:
#		{
#			set ::ExampleApp::InitFailed false
#			set ::ExampleApp::Initialized true 
#			break;
#		}
#
#		case Notification::Type_DriverReset:
#		case Notification::Type_MsgComplete:
#		case Notification::Type_NodeNaming:
#		case Notification::Type_NodeProtocolInfo:
#		case Notification::Type_NodeQueriesComplete:
#		default:
#		{
#		}
#	}
#
#	pthread_mutex_unlock( &g_criticalSection );
#



#
# Create the driver and then wait
#
proc ::ExampleApp::Main {} {

#       in the C++ version a mutex is used by the notifier thread
#       to let the main thread know initialization has been deteceted 
#       to be complete
#
#       in tcl we just set a tcl variable in the notifier callback ad use 
#       the tcl event system to wait for the variable to change via a tcl 
#       waitvar call in this main proc / thread (see the waitvar call below)
#       // pthread_mutex_lock( &initMutex );
#
 	## Create the OpenZWave Manager.
 	## The first argument is the path to the config files 
        ## (where the manufacturer_specific.xml file is located
 	## The second argument is the path for saved Z-Wave network state 
        ## and the log file.  If you leave it NULL 
 	## the log file will appear in the program's working directory.

 	ozw::options create \
            -configurationpath "../../../../config/" \
            -userpath "./" \
            -commandline ""

 	ozw::options addoptionint "SaveLogLevel" \
            [::ozw::log levelcode LogLevel_Detail]
 	ozw::options addoptionint "QueueLogLevel" \
            [::ozw::log levelcode LogLevel_Debug]
 	ozw::options addoptionint "DumpTrigger" \
            [::ozw::log levelcode LogLevel_Error]
 	ozw::options addoptionint "PollInterval" 500
 	ozw::options addoptionbool "IntervalBetweenPolls" true
 	ozw::options addoptionbool "ValidateValueChanges" true
 	ozw::options lock
 
 	ozw::manager create
 
 	## Add a callback handler to the manager.  
        ## The second argument is a context that
 	::ozw::manager addwatcher -command ::ExampleApp::OnNotification
 
 	## Add a Z-Wave Driver
 	## Modify this line to set the correct serial port for your 
        ## PC interface.

 	set port "/dev/cu.usbserial";
        ::ozw::log write LogLevel_Info "argv is $::argv"
 	if { $::argc > 0 } {
            set port [lindex $::argv 0]
 	} 
        
        if { $port == "usb"} {
            ## ozw::manager adddriver "HID Controller" Driver::ControllerInterface_Hid
 	} else {
            ozw::manager adddriver $port
 	}
 
        ##
	## Now we just wait for either the AwakeNodesQueried or
	## AllNodesQueried notification, then write out the config
	## file.  In a normal app, we would be handling notifications
	## and building a UI for the user.
        ##
        vwait ::ExampleApp::Initialized

	## Since the configuration file contains command class
	## information that is only known after the nodes on the network
	## are queried, wait until all of the nodes on the network
	## have been queried (at least the "listening" ones) before
	## writing the configuration file.  (Maybe write again after
	## sleeping nodes have been queried as well.)

 	if { $::ExampleApp::InitFailed } {
            ## ??
        } else {
 
            ::ozw::manager writeconfig ::ExampleApp::$HomeId;

            ## The section below demonstrates setting up polling for 
            ## a variable.  In this simple
            ## example, it has been hardwired to poll 
            ## COMMAND_CLASS_BASIC on the each node that 
            ## supports this setting.

#		pthread_mutex_lock( &g_criticalSection );
#		for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
#		{
#			NodeInfo* nodeInfo = *it;
#
#			// skip the controller (most likely node 1)
#			if( nodeInfo->m_nodeId == 1) continue;
#
#			for( list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2 )
#			{
#				ValueID v = *it2;
#				if( v.GetCommandClassId() == 0x20 )
#				{
#					Manager::Get()->EnablePoll( v, 2 );		// enables polling with "intensity" of 2, though this is irrelevant with only one value polled
#					break;
#				}
#			}
#		}
#		pthread_mutex_unlock( &g_criticalSection );
#
#		// If we want to access our NodeInfo list, that has been built from all the
#		// notification callbacks we received from the library, we have to do so
#		// from inside a Critical Section.  This is because the callbacks occur on other 
#		// threads, and we cannot risk the list being changed while we are using it.  
#		// We must hold the critical section for as short a time as possible, to avoid
#		// stalling the OpenZWave drivers.
#		// At this point, the program just waits for 3 minutes (to demonstrate polling),
#		// then exits
#		for( int i = 0; i < 60*3; i++ )
#		{
#			pthread_mutex_lock( &g_criticalSection );
#			// but NodeInfo list and similar data should be inside critical section
#			pthread_mutex_unlock( &g_criticalSection );
#			sleep(1);
#		}
#
#		Driver::DriverData data;
#		Manager::Get()->GetDriverStatistics( g_homeId, &data );
#		printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.s_SOFCnt, data.s_ACKWaiting, data.s_readAborts, data.s_badChecksum);
#		printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.s_readCnt, data.s_writeCnt, data.s_CANCnt, data.s_NAKCnt, data.s_ACKCnt, data.s_OOFCnt);
#		printf("Dropped: %d Retries: %d\n", data.s_dropped, data.s_retries);
#	}

        vwait forever

 	// program exit (clean up)
 	//if( strcasecmp( port.c_str(), "usb") == 0 ) {
        //Manager::Get()->RemoveDriver( "HID Controller" );
 	//} else {
 	//}
        ::ozw::manager removedriver $port
 	::ozw::manager removewatcher ::ExampleApp::OnNotification
 	::ozw::manager destroy
 	::ozw::options destroy

        # in tcl , the global mutex Ozw_MainMutex similar to 
        # g_criticalSection in the C++ example app it is internal 
        # to the tcl extension and is destroyed when the 
        # extension commands are destroyed in the tcl interpreter
 	#//pthread_mutex_destroy( &g_criticalSection );

        ::ozw::exit 0
 }

::ExampleApp::Main
