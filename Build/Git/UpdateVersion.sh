#!/bin/bash

cd "$(dirname "$0")"
FILE="../../Partitura/AcfSlnVoce.arp/VersionInfo.acc.xtrsvn"

git fetch --prune --unshallow 2>/dev/null

REV=$(git rev-list --count origin/main 2>/dev/null)
if [ -z "$REV" ]; then
    REV=$(git rev-list --count HEAD 2>/dev/null)
fi

if [ -z "$REV" ]; then
    echo "Failed to compute revision count."
    exit 1
fi

if git diff-index --quiet HEAD --; then
    DIRTY=0
else
    DIRTY=1
fi

echo "Git revision: $REV, dirty: $DIRTY"
echo "Processing file: $FILE"

OUT="${FILE%.xtrsvn}"
TMP="$OUT.tmp"

sed -e 's/\$WCREV\$/'"$REV"'/g' -e 's/\$WCMODS?1:0\$/'"$DIRTY"'/g' "$FILE" > "$TMP"

if [ -f "$OUT" ]; then
    if cmp -s "$TMP" "$OUT"; then
        rm -f "$TMP"
        echo "No changes in $OUT, file not rewritten."
    else
        mv -f "$TMP" "$OUT"
        echo "Wrote $OUT with WCREV=$REV and WCMODS=$DIRTY"
    fi
else
    mv -f "$TMP" "$OUT"
    echo "Wrote $OUT with WCREV=$REV and WCMODS=$DIRTY"
fi
