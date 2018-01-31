import qbs 1.0

Module
{
	name: 'ZeroMQ'

	Depends {name:'cpp'}
	cpp.dynamicLibraries: ['zmq']
}
