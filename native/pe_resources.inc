; PE resource directory. IDs match ibt.rc and the Windows UI code.
section .rsrc
pe_rsrc_start:
    dd 0, 0
    dw 0, 0, 0, 3
    dd 3, 0x80000000 | (pe_rsrc_icons - pe_rsrc_start)
    dd 14, 0x80000000 | (pe_rsrc_groups - pe_rsrc_start)
    dd 24, 0x80000000 | (pe_rsrc_manifests - pe_rsrc_start)

pe_rsrc_icons:
    dd 0, 0
    dw 0, 0, 0, 1
    dd 1, 0x80000000 | (pe_rsrc_icon_lang - pe_rsrc_start)
pe_rsrc_groups:
    dd 0, 0
    dw 0, 0, 0, 1
    dd 1, 0x80000000 | (pe_rsrc_group_lang - pe_rsrc_start)
pe_rsrc_manifests:
    dd 0, 0
    dw 0, 0, 0, 1
    dd 1, 0x80000000 | (pe_rsrc_manifest_lang - pe_rsrc_start)

%macro PE_RESOURCE_LANG 2
%1:
    dd 0, 0
    dw 0, 0, 0, 1
    dd 0x409, %2 - pe_rsrc_start
%endmacro
PE_RESOURCE_LANG pe_rsrc_icon_lang, pe_rsrc_icon_entry
PE_RESOURCE_LANG pe_rsrc_group_lang, pe_rsrc_group_entry
PE_RESOURCE_LANG pe_rsrc_manifest_lang, pe_rsrc_manifest_entry

%macro PE_RESOURCE_ENTRY 3
%1:
    dd %2 - PE_BASE, %3 - %2, 0, 0
%endmacro
PE_RESOURCE_ENTRY pe_rsrc_icon_entry, pe_rsrc_icon, pe_rsrc_icon_end
PE_RESOURCE_ENTRY pe_rsrc_group_entry, pe_rsrc_group, pe_rsrc_group_end
PE_RESOURCE_ENTRY pe_rsrc_manifest_entry, pe_rsrc_manifest, pe_rsrc_manifest_end

align 4, db 0
pe_rsrc_icon:
    incbin 'res/app.ico', 22
pe_rsrc_icon_end:
align 4, db 0
pe_rsrc_group:
    incbin 'res/app.ico', 0, 18
    dw 1
pe_rsrc_group_end:
align 4, db 0
pe_rsrc_manifest:
    incbin 'app.manifest'
pe_rsrc_manifest_end:
pe_rsrc_end:
