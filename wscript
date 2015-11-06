# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('capillary-network', ['core', 'network', 'mobility', 'spectrum', 'energy', 'applications'])
    module.source = [
        'model/capillary-phy.cc',
        'model/capillary-mac.cc',
        'model/capillary-controller.cc',
        'model/capillary-mac-header.cc',
        'model/capillary-mac-trailer.cc',
        'model/capillary-net-device.cc',
        'model/capillary-phy-ideal.cc',
        'model/energy-controller.cc',
        'model/basic-controller.cc',
        'model/residual-energy-controller.cc',
        'model/bounded-residual-energy-controller.cc',
        'model/fsaloha-mac.cc',
        'model/capillary-energy-model.cc',
        'model/capillary-tracer.cc',
        'model/sensor-application.cc',
        'model/bounded-energy-source.cc',
        'helper/capillary-energy-model-helper.cc',
        'helper/capillary-net-device-helper.cc',
        'helper/capillary-log-helper.cc',
        'helper/sensor-application-helper.cc',
        'helper/bounded-energy-source-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('capillary-network')
    module_test.source = [
        'test/fsaloha-packet-test.cc',
        'test/fsaloha-capillary-test.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'capillary-network'
    headers.source = [
        'model/capillary-phy.h',
        'model/capillary-mac.h',
        'model/capillary-controller.h',
        'model/capillary-mac-header.h',
        'model/capillary-mac-trailer.h',
        'model/capillary-net-device.h',
        'model/capillary-phy-ideal.h',
        'model/energy-controller.h',
        'model/basic-controller.h',
        'model/residual-energy-controller.h',
        'model/bounded-residual-energy-controller.h',
        'model/fsaloha-mac.h',
        'model/capillary-energy-model.h',
        'model/capillary-tracer.h',
        'model/sensor-application.h',
        'model/bounded-energy-source.h',
        'helper/capillary-energy-model-helper.h',
        'helper/capillary-net-device-helper.h',
        'helper/capillary-log-helper.h',  
        'helper/sensor-application-helper.h',
        'helper/bounded-energy-source-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

