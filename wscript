## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import ns3waf

def configure(conf):
    ns3waf.check_modules(conf, ['core', 'internet', 'point-to-point', 'csma' ], mandatory = True)

def build(bld):
    if bld.env['KERNEL_STACK']:
      bld.build_a_script('dce', needed = ['core', 'internet', 'dce', 'point-to-point', 'csma' ],
				  target='bin/test1',
				  source=['test1.cc'],
#				  linkflags=['-L/usr/local/lib'],
#				  lib=['foolib']
				  )


