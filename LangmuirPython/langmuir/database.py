# -*- coding: utf-8 -*-
import numpy as _np

class ColumnMetaData:
    """
    """
    def __init__(self, name, key, pytype, dflt, units, fmt, calculated):
        """
        """
        self.name       = name
        self.key        = key
        self.pytype     = pytype
        self.dflt       = dflt
        self.units      = units
        self.fmt        = fmt
        self.calculated = calculated
        self._calc_nptype()

    def _calc_nptype(self, string_size=16):
        """
        """
        if self.pytype == float:
            self.nptype = _np.float64
        elif self.pytype == int:
            self.nptype = _np.int64
        elif self.pytype == bool:
            self.nptype = _np.bool
        elif self.pytype == str:
            self.nptype = '|S%d' % string_size
        else:
            self.nptype = self.pytype

    def __repr__(self):
        """
        """
        return 'Column(name=%s)' % self.name

class ColumnList:
    """
    """
    def __init__(self):
        """
        """
        self.columns = []

    def append(self, column):
        """
        """
        self.columns.append(column)

    def __getitem__(self, index):
        """
        """
        if isinstance(index, str):
            index = self.names.index(index)
        return self.columns[index]

    def _init(self):
        """
        """
        self.names = [col.name for col in self.columns]
        self.keys = [col.key for col in self.columns]
        self.pytypes = [col.pytype for col in self.columns]
        self.dflts = [col.dflt for col in self.columns]
        self.units = [col.units for col in self.columns]
        self.fmts = [col.fmt for col in self.columns]
        self.calculated = [col.calculated for col in self.columns]
        self.dtype = []
        for col in self.columns:
            self.dtype.append((col.name, col.nptype))
        self.dtype = _np.dtype(self.dtype)
        self.size = len(self.columns)

parameters = ColumnList()
parameters.append(ColumnMetaData('simulation.type'           , 'simulation_type'           ,   str,  'solarcell',       None,     '%23s',  False))
parameters.append(ColumnMetaData('current.step'              , 'current_step'              ,   int,            0,       'ps',     '%23d',  False))
parameters.append(ColumnMetaData('iterations.real'           , 'iteration_real'            ,   int,        10000,       'ps',     '%23d',  False))
parameters.append(ColumnMetaData('random.seed'               , 'random_seed'               ,   int,            0,       None,     '%23d',  False))
parameters.append(ColumnMetaData('grid.z'                    , 'grid_z'                    ,   int,            1,       'nm',     '%23d',  False))
parameters.append(ColumnMetaData('grid.y'                    , 'grid_y'                    ,   int,          256,       'nm',     '%23d',  False))
parameters.append(ColumnMetaData('grid.x'                    , 'grid_x'                    ,   int,          256,       'nm',     '%23d',  False))
parameters.append(ColumnMetaData('hopping.range'             , 'hopping_range'             ,   int,            2,       'nm',     '%23d',  False))
parameters.append(ColumnMetaData('output.is.on'              , 'output_is_on'              ,  bool,         True,       None,     '%23s',  False))
parameters.append(ColumnMetaData('iterations.print'          , 'iteration_print'           ,   int,         1000,       'ps',     '%23d',  False))
parameters.append(ColumnMetaData('output.precision'          , 'output_precision'          ,   int,           15,       None,     '%23d',  False))
parameters.append(ColumnMetaData('output.width'              , 'output_width'              ,   int,           23,       None,     '%23d',  False))
parameters.append(ColumnMetaData('output.stub'               , 'output_stub'               ,   str,        'out',       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.ids.on.delete'      , 'output_ids_on_delete'      ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.ids.on.encounter'   , 'output_ids_on_encounter'   ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.coulomb'            , 'output_coulomb'            ,   int,            0,       None,     '%23d',  False))
parameters.append(ColumnMetaData('output.step.chk'           , 'output_step_chk'           ,   int,           10,       None,     '%23d',  False))
parameters.append(ColumnMetaData('output.chk.trap.potential' , 'output_chk_trap_potential' ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.potential'          , 'output_potential'          ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.xyz'                , 'output_xyz'                ,   int,            0,       None,     '%23d',  False))
parameters.append(ColumnMetaData('output.xyz.e'              , 'output_xyz_e'              ,  bool,         True,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.xyz.h'              , 'output_xyz_h'              ,  bool,         True,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.xyz.d'              , 'output_xyz_d'              ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.xyz.t'              , 'output_xyz_t'              ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('output.xyz.mode'           , 'output_xyz_mode'           ,   int,            0,       None,     '%23d',  False))
parameters.append(ColumnMetaData('image.traps'               , 'image_traps'               ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('image.defects'             , 'image_defects'             ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('image.carriers'            , 'image_carriers'            ,   int,            0,       None,     '%23d',  False))
parameters.append(ColumnMetaData('electron.percentage'       , 'electron_percentage'       , float,         0.01,        '%',  '%23.15e',  False))
parameters.append(ColumnMetaData('hole.percentage'           , 'hole_percentage'           , float,         0.01,        '%',  '%23.15e',  False))
parameters.append(ColumnMetaData('seed.charges'              , 'seed_charges'              , float,          0.0,       None,  '%23.15e',  False))
parameters.append(ColumnMetaData('defect.percentage'         , 'defect_percentage'         , float,          0.0,        '%',  '%23.15e',  False))
parameters.append(ColumnMetaData('trap.percentage'           , 'trap_percentage'           , float,          0.0,        '%',  '%23.15e',  False))
parameters.append(ColumnMetaData('trap.potential'            , 'trap_potential'            , float,          0.0,        'V',  '%23.15e',  False))
parameters.append(ColumnMetaData('gaussian.stdev'            , 'gaussian_stdev'            , float,          0.0,        'V',  '%23.15e',  False))
parameters.append(ColumnMetaData('seed.percentage'           , 'seed_percentage'           , float,          0.0,        '%',  '%23.15e',  False))
parameters.append(ColumnMetaData('voltage.right'             , 'voltage_right'             , float,          0.0,        'V',  '%23.15e',  False))
parameters.append(ColumnMetaData('voltage.left'              , 'voltage_left'              , float,          0.0,        'V',  '%23.15e',  False))
parameters.append(ColumnMetaData('exciton.binding'           , 'exciton_binding'           , float,          0.0,        'V',  '%23.15e',  False))
parameters.append(ColumnMetaData('slope.z'                   , 'slope_z'                   , float,          0.0,     'V/nm',  '%23.15e',  False))
parameters.append(ColumnMetaData('coulomb.carriers'          , 'coulomb_carriers'          ,  bool,         True,       None,     '%23s',  False))
parameters.append(ColumnMetaData('coulomb.gaussian.sigma'    , 'coulomb_gaussian_sigma'    , float,          1.0,       'nm',  '%23.15e',  False))
parameters.append(ColumnMetaData('defects.charge'            , 'defects_charge'            ,   int,            0,        'e',     '%23d',  False))
parameters.append(ColumnMetaData('temperature.kelvin'        , 'temperature_kelvin'        , float,        300.0,        'K',  '%23.15e',  False))
parameters.append(ColumnMetaData('source.rate'               , 'source_rate'               , float,        0.001,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('e.source.l.rate'           , 'e_source_l_rate'           , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('e.source.r.rate'           , 'e_source_r_rate'           , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('h.source.l.rate'           , 'h_source_l_rate'           , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('h.source.r.rate'           , 'h_source_r_rate'           , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('generation.rate'           , 'generation_rate'           , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('balance.charges'           , 'balance_charges'           ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('source.metropolis'         , 'source_metropolis'         ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('source.coulomb'            , 'source_coulomb'            ,  bool,        False,       None,     '%23s',  False))
parameters.append(ColumnMetaData('source.scale.area'         , 'source_scale_area'         , float,      65536.0,    'nm**2',  '%23.15e',  False))
parameters.append(ColumnMetaData('drain.rate'                , 'drain_rate'                , float,          0.9,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('e.drain.l.rate'            , 'e_drain_l_rate'            , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('e.drain.r.rate'            , 'e_drain_r_rate'            , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('h.drain.l.rate'            , 'h_drain_l_rate'            , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('h.drain.r.rate'            , 'h_drain_r_rate'            , float,         -1.0,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('recombination.rate'        , 'recombination_rate'        , float,       0.0001,     '1/ps',  '%23.15e',  False))
parameters.append(ColumnMetaData('recombination.range'       , 'recombination_range'       ,   int,            0,       'nm',     '%23d',  False))
parameters.append(ColumnMetaData('use.opencl'                , 'use_opencl'                ,  bool,         True,       None,     '%23s',  False))
parameters.append(ColumnMetaData('work.x'                    , 'work_x'                    ,   int,            4,   'groups',     '%23d',  False))
parameters.append(ColumnMetaData('work.y'                    , 'work_y'                    ,   int,            4,   'groups',     '%23d',  False))
parameters.append(ColumnMetaData('work.z'                    , 'work_z'                    ,   int,            4,   'groups',     '%23d',  False))
parameters.append(ColumnMetaData('work.size'                 , 'work_s'                    ,   int,          256,  'workers',     '%23d',  False))
parameters.append(ColumnMetaData('opencl.threshold'          , 'opencl_threshold'          ,   int,          256,  'charges',     '%23d',  False))
parameters.append(ColumnMetaData('opencl.device.id'          , 'opencl_device_id'          ,   int,            0,       None,     '%23d',  False))
parameters.append(ColumnMetaData('max.threads'               , 'max_threads'               ,   int,           -1,  'threads',     '%23d',  False))
parameters._init()

output = ColumnList()
output.append(ColumnMetaData('simulation:time'           , 'time'                      ,   int,            0,       'ps',     '%23d',  False))
output.append(ColumnMetaData('eSourceL:attempt'          , 'esl_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eSourceL:success'          , 'esl_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eSourceL:prob'             , 'esl_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('eSourceL:rate'             , 'esl_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('eSourceL:current'          , 'esl_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('eSourceL:density'          , 'esl_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('eSourceL:power'            , 'esl_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('eSourceL:irradiance'       , 'esl_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('eSourceR:attempt'          , 'esr_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eSourceR:success'          , 'esr_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eSourceR:prob'             , 'esr_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('eSourceR:rate'             , 'esr_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('eSourceR:current'          , 'esr_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('eSourceR:density'          , 'esr_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('eSourceR:power'            , 'esr_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('eSourceR:irradiance'       , 'esr_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('hSourceL:attempt'          , 'hsl_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hSourceL:success'          , 'hsl_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hSourceL:prob'             , 'hsl_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('hSourceL:rate'             , 'hsl_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('hSourceL:current'          , 'hsl_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('hSourceL:density'          , 'hsl_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('hSourceL:power'            , 'hsl_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('hSourceL:irradiance'       , 'hsl_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('hSourceR:attempt'          , 'hsr_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hSourceR:success'          , 'hsr_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hSourceR:prob'             , 'hsr_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('hSourceR:rate'             , 'hsr_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('hSourceR:current'          , 'hsr_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('hSourceR:density'          , 'hsr_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('hSourceR:power'            , 'hsr_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('hSourceR:irradiance'       , 'hsr_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('xSource:attempt'           , 'xs_attempt'                ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('xSource:success'           , 'xs_success'                ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('xSource:prob'              , 'xs_probability'            , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('xSource:rate'              , 'xs_rate'                   , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('xSource:current'           , 'xs_current'                , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('xSource:density'           , 'xs_density'                , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('xSource:power'             , 'xs_power'                  , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('xSource:irradiance'        , 'xs_irradiance'             , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('eDrainL:attempt'           , 'edl_attempt'               , float,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eDrainL:success'           , 'edl_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eDrainL:prob'              , 'edl_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('eDrainL:rate'              , 'edl_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('eDrainL:current'           , 'edl_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('eDrainL:density'           , 'edl_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('eDrainL:power'             , 'edl_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('eDrainL:irradiance'        , 'edl_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('eDrainR:attempt'           , 'edr_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eDrainR:success'           , 'edr_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('eDrainR:prob'              , 'edr_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('eDrainR:rate'              , 'edr_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('eDrainR:current'           , 'edr_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('eDrainR:density'           , 'edr_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('eDrainR:power'             , 'edr_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('eDrainR:irradiance'        , 'edr_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('hDrainL:attempt'           , 'hdl_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hDrainL:success'           , 'hdl_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hDrainL:prob'              , 'hdl_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('hDrainL:rate'              , 'hdl_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('hDrainL:current'           , 'hdl_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('hDrainL:density'           , 'hdl_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('hDrainL:power'             , 'hdl_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('hDrainL:irradiance'        , 'hdl_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('hDrainR:attempt'           , 'hdr_attempt'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hDrainR:success'           , 'hdr_success'               ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('hDrainR:prob'              , 'hdr_probability'           , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('hDrainR:rate'              , 'hdr_rate'                  , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('hDrainR:current'           , 'hdr_current'               , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('hDrainR:density'           , 'hdr_density'               , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('hDrainR:power'             , 'hdr_power'                 , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('hDrainR:irradiance'        , 'hdr_irradiance'            , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('xDrain:attempt'            , 'xd_attempt'                ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('xDrain:success'            , 'xd_success'                ,   int,            0,   'counts',     '%23d',  False))
output.append(ColumnMetaData('xDrain:prob'               , 'xd_probability'            , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('xDrain:rate'               , 'xd_rate'                   , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('xDrain:current'            , 'xd_current'                , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('xDrain:density'            , 'xd_density'                , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('xDrain:power'              , 'xd_power'                  , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('xDrain:irradiance'         , 'xd_irradiance'             , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('electron:rate'             , 'e_rate'                    , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('electron:current'          , 'e_current'                 , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('electron:density'          , 'e_density'                 , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('electron:power'            , 'e_power'                   , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('electron:irradiance'       , 'e_irradiance'              , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('hole:rate'                 , 'h_rate'                    , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('hole:current'              , 'h_current'                 , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('hole:density'              , 'h_density'                 , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('hole:power'                , 'h_power'                   , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('hole:irradiance'           , 'h_irradiance'              , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('drain:rate'                , 'd_rate'                    , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('drain:current'             , 'd_current'                 , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('drain:density'             , 'd_density'                 , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('drain:power'               , 'd_power'                   , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('drain:irradiance'          , 'd_irradiance'              , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('source:rate'               , 's_rate'                    , float,          0.0,     '1/ps',  '%23.15e',   True))
output.append(ColumnMetaData('source:current'            , 's_current'                 , float,          0.0,       'nA',  '%23.15e',   True))
#output.append(ColumnMetaData('source:density'            , 's_density'                 , float,          0.0, 'mA/cm**2',  '%23.15e',   True))
#output.append(ColumnMetaData('source:power'              , 's_power'                   , float,          0.0,       'nW',  '%23.15e',   True))
#output.append(ColumnMetaData('source:irradiance'         , 's_irradiance'              , float,          0.0, 'mW/cm**2',  '%23.15e',   True))
output.append(ColumnMetaData('electron:count'            , 'e_count'                   ,   int,            0,   'counts',     '%23d',  False))
#output.append(ColumnMetaData('electron:percentage'       , 'e_percent'                 , float,          0.0,        '%',  '%23.15e',   True))
#output.append(ColumnMetaData('electron:reached'          , 'e_reached'                 , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('hole:count'                , 'h_count'                   ,   int,            0,   'counts',     '%23d',  False))
#output.append(ColumnMetaData('hole:percentage'           , 'h_percent'                 , float,          0.0,        '%',  '%23.15e',   True))
#output.append(ColumnMetaData('hole:reached'              , 'h_reached'                 , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('carrier:count'             , 'c_count'                   ,   int,            0,   'counts',     '%23d',   True))
#output.append(ColumnMetaData('carrier:percentage'        , 'c_percent'                 , float,          0.0,        '%',  '%23.15e',   True))
#output.append(ColumnMetaData('carrier:reached'           , 'c_reached'                 , float,          0.0,        '%',  '%23.15e',   True))
output.append(ColumnMetaData('carrier:difference'        , 'c_difference'              ,   int,            0,   'counts',     '%23d',   True))
output.append(ColumnMetaData('real:time'                 , 'real'                      ,   int,            0,       'ms',     '%23d',  False))
output.append(ColumnMetaData('part'                      , 'part'                      ,   int,            0,         '',     '%23d',   True))
output._init()

fluxes = []
fluxes.append('eSourceL')
fluxes.append('eSourceR')
fluxes.append('hSourceL')
fluxes.append('hSourceR')
fluxes.append('eDrainL')
fluxes.append('eDrainR')
fluxes.append('hDrainL')
fluxes.append('hDrainR')
fluxes.append('xSource')
fluxes.append('xDrain')

try:
    import quantities as _units
    cfactor = float((_units.e / _units.ps).rescale(_units.nA))
except:
    cfactor = 160.21764869999996