{
    'targets': [
        {
            'target_name': 'magic',
            'sources': [
                'src/magic.cc'
            ],
            'include_dirs': [
                '.',
                '/usr/include'
            ],
            'cflags': [],
            'link_settings': {
                'libraries': [
                    '-lmagic'
                ],
                'include_dirs': [
                    '/usr/include'
                ]
            }
        }
    ]
}
