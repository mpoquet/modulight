import qbs 1.0

Project
{
	DynamicLibrary
	{
		name: 'modulight'

		Depends { name: 'cpp' }

		Depends { name: 'OpenMPI' }
		Depends { name: 'zmq' }
		Depends { name: 'Qt.core' }
		Depends { name: 'Qt.network' }
		Depends { name: 'Qt.xml' }

		cpp.includePaths: ['include']

		Export
		{
			Depends { name: "cpp" }
			Depends { name: 'OpenMPI' }
			Depends { name: 'zmq' }
			Depends { name: 'Qt.core' }
			Depends { name: 'Qt.network' }
			Depends { name: 'Qt.xml' }
			
			cpp.includePaths : ['include']
		}

		files:
		[
			'include/modulight/common/network.hpp',
			'include/modulight/common/sequence.hpp',
			'include/modulight/common/xml.hpp',
			'include/modulight/common/tag.hpp',
			'include/modulight/common/arguments.hpp',
			'include/modulight/common/mpiutils.hpp',
			'include/modulight/common/dynamicorder.hpp',
			'include/modulight/common/dynamicrequest.hpp',
			'include/modulight/common/dot.hpp',
			'include/modulight/common/moduledescription.hpp',
			'include/modulight/common/modulightexception.hpp',

			'include/modulight/master/masterconnection.hpp',
			'include/modulight/master/userinterface.hpp',
			'include/modulight/master/process.hpp',
			'include/modulight/master/argumenthandler.hpp',
			'include/modulight/master/hostfile.hpp',
			'include/modulight/master/reachableexecutables.hpp',

			'include/modulight/module/messagereader.hpp',
			'include/modulight/module/messagewriter.hpp',
			'include/modulight/module/modulestate.hpp',
			'include/modulight/module/stamp.hpp',
			'include/modulight/module/port.hpp',

			'include/modulight/application.hpp',
			'include/modulight/module.hpp',


			'src/common/mpiutils.cpp',
			'src/common/arguments.cpp',
			'src/common/dot.cpp',
			'src/common/network.cpp',
			'src/common/xml.cpp',

			'src/master/masterconnection.cpp',
			'src/master/reachableexecutables.cpp',
			'src/master/process.cpp',
			'src/master/hostfile.cpp',
			'src/master/argumenthandler.cpp',
			'src/master/application.cpp',

			'src/module/module.cpp',
			'src/module/messagereader.cpp',
			'src/module/stamp.cpp',
			'src/module/messagewriter.cpp'
		]
	}
}