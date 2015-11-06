# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('capillary-aloha', ['core', 'network', 'mobility', 'spectrum', 'energy', 'applications', 'capillary-network'])
    module.source = [
		'model/fsaloha-mac.cc',
		'model/capillary-tracer.cc',
		'model/basic-controller.cc',
		'model/capillary-phy-ideal.cc',
		'model/residual-energy-controller.cc',
		'model/bounded-energy-source.cc',
        'helper/bounded-energy-source-helper.cc',
        'helper/capillary-log-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('capillary-aloha')
    module_test.source = [
        'test/capillary-fsaloha-test.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'capillary-aloha'
    headers.source = [
    	'model/capillary-tracer.h',
		'model/basic-controller.h',
		'model/capillary-phy-ideal.h',
		'model/residual-energy-controller.h',
        'model/fsaloha-mac.h',
        'model/bounded-energy-source.h',
        'helper/bounded-energy-source-helper.h',
        'helper/capillary-log-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

