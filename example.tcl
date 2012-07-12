#--------------------------------------------------------------------------
#	example.tcl
#	tcl version of Minimal application to test OpenZWave.
#--------------------------------------------------------------------------
puts stderr "ozw package: [package require ozw]"
namespace eval ::ExampleApp {}

set ::ExampleApp::Initialized false
set ::ExampleApp::InitFailed false


##
## at the moment, ## the tcl extensions assume that you will define 
##
## proc ::ozw::notificationacceptor {sock addr port} {...}
## proc ::ozw::notificationreader {sock addr port} {...}
## as procedures for handling incoming messaged from the thread
## handling the C++ level notification callbacks 
##
## messags will be coming in in the fornm
##
##      "notification {type x nodeid n ...}\n"
##
proc ::ozw::notificationacceptor {sock addr port} {
     fconfigure $sock -buffering line
     fileevent $sock readable [list ozw::notificationreader $sock]
}

set ::ozw::notificationreaderbuffer {}
proc ::ozw::notificationreader {sock} {
    set nm reader; set lvl LogLevel_Info
    if {[eof $sock]} {
        ::ozw::log write $lvl \
            "$nm: End of file on notification. closing channel $sock"
        close $sock
        return
    }
    set failed [catch {gets $sock data} err]
    if $failed {
        ::ozw::log write $lvl \
            "$nm: Read error on notification. closing channel $sock"
        close $sock
        return
    }
    if {[info complete [append ::ozw::notificationreaderbuffer $data]]} {
        if {[llength $::ozw::notificationreaderbuffer] % 2 != 0} {
            ::ozw::log write $lvl \
            "$nm: Makformed message buffer \"$::ozw::notificationreaderbuffer\""
            set ::ozw::notificationreaderbuffer {}
        }
        set failed [catch {
            foreach {tag data} $::ozw::notificationreaderbuffer {
                $::ozw::notificationreaderproc $tag $data
            }
        } err]
        if {$failed} {
            ::ozw::log write $lvl \
                "$nm: Error processing message [list $tag $data]"
            ::ozw::log write $lvl \
                "$nm: errorInfo: $::errorInfo"
        }
        set ::ozw::notificationreaderbuffer {}
    }
}

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
proc ::ExampleApp::UpdateNodeInfo {notification args} {
    set key [$notification cget -homeid].[$notification cget -nodeid]
    if {[info exists ::ExampleApp::Nodes($key)]} {
        array set a ::ExampleApp::Nodes($key)
        array set a $args
        set ::ExampleApp::Nodes($key) [array get a]
        ::ExampleApp::Log \
            "updated node $key with: $args: it's now [array get a]"
        return true
    } else {
        ::ExampleApp::Log "could not find node info w/ key ( $key ) "
        return false
    }
    return false
}
proc ::ExampleApp::RemoveNodeInfo {notification} {
    set key [$notification cget -homeid].[$notification cget -nodeid]
    if {[info exists ::ExampleApp::Nodes($key)]} {
        unset ::ExampleApp::Nodes($key)
        return true
    } else {
        ::ExampleApp::Log "could not find node info w/ key ( $key ) "
        return false
    }
    return false
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
proc ::ExampleApp::GetNodeInfo {n} {
    upvar $n notification
    set key $notification(homeid).$notification(nodeid)
    if {[info exists ::ExampleApp::Nodes($key)]} {
        return $::ExampleApp::Nodes($key)
    }
    return ""
}
proc ::ExampleApp::NodeInfoAddValue {notification value} {
    upvar $n notification
    set key $notification(homeid).$notification(nodeid)
    if {[info exists ::ExampleApp::Nodes($key)]} {
        array set a $::ExampleApp::Nodes($key)
        lappend a(-values) $value
        set ::ExampleApp::Nodes($key) [array get a]
        return true
    } else {
        ::ExampleApp::Log "could not find node info w/ key ( $key ) "
        return false
    }
    return false
}
proc ::ExampleApp::NodeInfoRemoveValue {notification value} {
    upvar $n notification
    set key $notification(homeid).$notification(nodeid)
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
    return false
}

proc ::ExampleApp::Log {msg} {
    ::ozw::log write LogLevel_Info "ExampleApp: ** $msg"
}
 

proc ::ExampleApp::HandleNotification {tag data} {
    array set n $data

    set typestring $n(type)
    ::ExampleApp::Log " @@ NotificationHandler ==> $typestring"
    switch -exact -- $typestring Type_ValueAdded {
        if {[::ExampleApp::GetNodeInfo $notification] != ""} {
            ::ExampleApp::NodeInfoAddValue n $n(valueid)
        }
    } Type_ValueRemoved {
        if {[::ExampleApp::GetNodeInfo $notification] != ""} {
            ::ExampleApp::NodeInfoRemoveValue n $n(valueid)
        }
    } Type_ValueChanged {
        ## One of the node values has changed
        if {[set n [::ExampleApp::GetNodeInfo $notification]] != ""} {
            ::ExampleApp::Log "Node Value for $data changed"
        }
    } Type_Group {
        ## One of the node's association groups has changed
        if {[set ni [::ExampleApp::GetNodeInfo n]] != ""} {
            ::ExampleApp::Log "Node assoc. groups for $ni changed"
        }
    } Type_NodeAdded {
        ::ExampleApp::AddNodeInfo $n(homeid) $n(nodeid) false {}
    } Type_NodeRemoved {
        ::ExampleApp::RemoveNodeInfo n
    } Type_NodeEvent {
        ## We have received an event from the node, caused by a
        ## basic_set or hail message.
        ::ExampleApp::Log \
            "Recvd event from node. caused by a basic_set or hail msg"
    } Type_PollingDisabled  {
        ::ExampleApp::UpdateNodeInfo n -polled false
    } Type_PollingEnabled {
        ::ExampleApp::UpdateNodeInfo n -polled true
    } Type_DriverReady {
        set ::ExampleApp::HomeId $n(homeid)
        ::ExampleApp::Log "Driver Ready: home id = $::ExampleApp::HomeId" 
    } Type_DriverFailed {
        set ::ExampleApp::InitFailed true
        set ::ExampleApp::Initialized true 
        ::ExampleApp::Log "set ::ExampleApp::Initialized true"
    } Type_AwakeNodesQueried - Type_AllNodesQueried {
        set ::ExampleApp::InitFailed false
        set ::ExampleApp::Initialized true 
        ::ExampleApp::Log "set ::ExampleApp::Initialized true"
    } Type_DriverReset - \
      Type_MsgComplete - \
      Type_NodeNaming - \
      Type_NodeProtocolInfo - \
      Type_NodeQueriesComplete - \
      default {
        ::ExampleApp::Log "$typestring: (no action for this event type)"
    }
}

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
        ## for the notification callbacks. note that in this tcl library
        ## the notifier thread sends notification messages via a socket to 
        ## the main thread (on the local 127.0.0.0 address) so one can write 
        ## a familiar tcl event driven program
        ## without dealing with all kinds of thread drama
        ##
        ## add watcher automatically creates the socket connection and then
        ## installs -command as the handler for incoming messages
        ## the server/recv portion of this message is created in 
        ## ozw::manager adddriver. the client/send socket is created at this
        ## time also. the actual connect is deferred until the first C++ level
        ## callback happens. the connect is then peformed to the recv side
        ## and the notification is sent as a message to the recv side 
        ## (main thread) which then executed the designated -command along 
        ## with the notification message as a trailing argument.
        ## the message is a {tag value ... tag value} list suitable for 
        ## use as a tcl array via [array set $message]
 	::ozw::manager addwatcher -command ::ExampleApp::HandleNotification 
 	## Add a Z-Wave Driver
 	## Modify this line to set the correct serial port for your 
        ## PC interface.

 	set device "/dev/cu.usbserial";
        ::ozw::log write LogLevel_Info "argv is $::argv"
 	if { $::argc > 0 } {
            set device [lindex $::argv 0]
 	} 
        
        if { $device == "usb"} {
            ## ozw::manager adddriver "HID Controller" Driver::ControllerInterface_Hid
 	} else {
            ozw::manager adddriver $device
 	}

        ##
	## Now we just wait for either the AwakeNodesQueried or
	## AllNodesQueried notification, then write out the config
	## file.  In a normal app, we would be handling notifications
	## and building a UI for the user.
        ##

	## Since the configuration file contains command class
	## information that is only known after the nodes on the network
	## are queried, wait until all of the nodes on the network
	## have been queried (at least the "listening" ones) before
	## writing the configuration file.  (Maybe write again after
	## sleeping nodes have been queried as well.)

        if {$::ExampleApp::Initialized == false} {
            vwait ::ExampleApp::Initialized
        }

 	if { $::ExampleApp::InitFailed } {
            ## ??
        } else {
            ::ExampleApp::Log "Driver Ready: home id = $::ExampleApp::HomeId" 
            ::ozw::manager writeconfig $::ExampleApp::HomeId;
            ::ExampleApp::Log "wrote configuration. (initialization done)"

            ## The section below demonstrates setting up polling for 
            ## a variable.  In this simple
            ## example, it has been hardwired to poll 

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
        ::ozw::manager removedriver $device
 	::ozw::manager removewatcher -command ::ExampleApp::HandleNotification 
 	::ozw::manager destroy
 	::ozw::options destroy

        # in tcl , the global mutex Ozw_MainMutex similar to 
        # g_criticalSection in the C++ example app it is internal 
        # to the tcl extension and is destroyed when the 
        # extension commands are destroyed in the tcl interpreter
 	#//pthread_mutex_destroy( &g_criticalSection );
}

::ExampleApp::Main
vwait forever
