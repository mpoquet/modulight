import qbs 1.0

Module
{
	name: 'modulight'

	Depends { name: 'cpp' }
	Depends { name: 'OpenMPI' }
	Depends { name: 'zmq' }
	Depends { name: 'Qt.core' }
	Depends { name: 'Qt.network' }
	Depends { name: 'Qt.xml' }

	cpp.dynamicLibraries : ['modulight']
}
