#!/usr/bin/env bash
# Read only the documented keys; never execute the player's configuration.
LoadMultiplayerConfig()
{
    local config_path="$1" key value octet
    local -a octets
    if [[ -f "$config_path" ]]; then
        while IFS='=' read -r key value || [[ -n "$key" ]]; do
            value="${value%$'\r'}"
            case "$key" in
                ''|\#*) continue ;;
                POKE_NET_SERVER)
                    if [[ ! ${POKE_NET_SERVER+x} ]]; then export POKE_NET_SERVER="$value"; fi ;;
                POKE_NET_PORT)
                    if [[ ! ${POKE_NET_PORT+x} ]]; then export POKE_NET_PORT="$value"; fi ;;
                *) echo 'Configuração multiplayer contém uma chave desconhecida.' >&2; return 1 ;;
            esac
        done < "$config_path"
    fi
    if [[ ! ${POKE_NET_SERVER:-} =~ ^[0-9]{1,3}(\.[0-9]{1,3}){3}$ ]]; then
        echo 'Preencha POKE_NET_SERVER com o IPv4 do seu servidor na configuração local.' >&2
        return 1
    fi
    IFS='.' read -r -a octets <<< "$POKE_NET_SERVER"
    for octet in "${octets[@]}"; do
        if (( 10#$octet > 255 )); then echo 'IPv4 inválido.' >&2; return 1; fi
    done
    if [[ ! ${POKE_NET_PORT:-} =~ ^[0-9]{1,5}$ ]] || (( 10#$POKE_NET_PORT < 1 || 10#$POKE_NET_PORT > 65535 )); then
        echo 'Preencha POKE_NET_PORT com uma porta entre 1 e 65535.' >&2
        return 1
    fi
    printf -v POKE_NET_SERVER '%d.%d.%d.%d' "$((10#${octets[0]}))" "$((10#${octets[1]}))" "$((10#${octets[2]}))" "$((10#${octets[3]}))"
    POKE_NET_PORT="$((10#$POKE_NET_PORT))"
    export POKE_NET_SERVER POKE_NET_PORT
}
