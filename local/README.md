Machine-local overrides. Anything dropped here as a .yaml file is merged into
every device config after packages/common.yaml, so it wins.

The directory is gitignored, and an empty one is fine - ESPHome's
!include_dir_merge_named yields nothing rather than failing.

    # local/ota.yaml
    substitutions:
      ota_password: "something-else"
