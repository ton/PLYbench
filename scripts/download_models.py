#!/usr/bin/env python

import gzip
import hashlib
import os
import shutil
import sys
import tarfile
import urllib.request

from pathlib import Path
from zipfile import ZipFile

class Model:
    def __init__(self, name, archive_url, archive_name = None, archive_ply_filename = None):
        self.name = name
        self.archive_url = archive_url
        self.archive_filename = archive_name if archive_name is not None else os.path.basename(archive_url)

        if archive_ply_filename is not None:
            self.archive_ply_filename = archive_ply_filename
            self.ply_filename = os.path.basename(self.archive_ply_filename)
        else:
            self.archive_ply_filename = None
            self.ply_filename = os.path.basename(self.archive_url)
            if self.ply_filename.endswith('.gz'):
                self.ply_filename = Path(self.ply_filename).stem

    def download(self, base_url, target_directory):
        sys.stdout.write(f"> Downloading {self.name} from {base_url}/{self.archive_url}...")
        sys.stdout.flush()

        archive_filename = os.path.join(target_directory, self.archive_filename)
        archive_md5sum_filename = archive_filename + '.md5sum'
        ply_filename = os.path.join(target_directory, self.ply_filename)

        md5sum_matches = os.path.exists(archive_filename) and os.path.exists(archive_md5sum_filename) and \
                hashlib.md5(open(archive_filename, 'rb').read()).hexdigest() == open(archive_md5sum_filename).read()

        if md5sum_matches:
            sys.stdout.write("local model archive is up to date\n")
        else:
            # Download the model archive.
            urllib.request.urlretrieve(base_url + '/' + self.archive_url, archive_filename)
            sys.stdout.write("done\n")

            # Write MD5 sum of the model archive.
            with open(archive_md5sum_filename, 'w') as archive_md5sum_file:
                archive_md5sum_file.write(hashlib.md5(open(archive_filename, 'rb').read()).hexdigest())

        # In case the local PLY file does not exist, extract it.
        if not os.path.exists(ply_filename):
            if archive_filename.endswith('.tar.gz') and archive_ply_filename is not None:
                with tarfile.open(archive_filename, 'r') as archive_file:
                    member = archive_file.getmember(self.archive_ply_filename)
                    member.name = ply_filename
                    archive_file.extract(member)
            elif archive_filename.endswith('.gz'):
                with gzip.open(archive_filename, 'rb') as gzip_file:
                    with open(ply_filename, 'wb') as ply_file:
                        shutil.copyfileobj(gzip_file, ply_file)
            elif archive_filename.endswith('.zip'):
                with ZipFile(archive_filename, 'r') as zip_file:
                    with zip_file.open(self.archive_ply_filename) as zipped_ply_file:
                        with open(ply_filename, 'wb') as ply_file:
                            shutil.copyfileobj(zipped_ply_file, ply_file)

# Download Stanford models.
stanford_url = 'http://graphics.stanford.edu'

stanford_models = [Model('Stanford Bunny', 'bunny.tar.gz', archive_ply_filename='pub/3Dscanrep/bunny/reconstruction/bun_zipper.ply'),
                   Model('Happy Buddha', 'happy/happy_recon.tar.gz', archive_ply_filename='pub/3Dscanrep/happy_recon/happy_vrip.ply'),
                   Model('Dragon', 'dragon/dragon_recon.tar.gz', archive_ply_filename='pub/3Dscanrep/dragon_recon/dragon_vrip.ply'),
                   Model('Asian Dragon', 'data/3Dscanrep/xyzrgb/xyzrgb_dragon.ply.gz'),
                   Model('Lucy', 'data/3Dscanrep/lucy.tar.gz', archive_ply_filename='lucy.ply')]

for model in stanford_models:
    model.download(stanford_url, 'models/')

artec_url = 'https://cdn.artec3d.com/content-hub-3dmodels'
artec_models = [Model('Doom Combat Scene', 'doom-combat-scene_ply.zip?VersionId=5N9bHbcyQDcBcQNbkdxjgZws2f4d3Es_', archive_name='doom-combat-scene_ply.zip', archive_ply_filename='Doom combat scene.ply')]

for model in artec_models:
    model.download(artec_url, 'models/')
