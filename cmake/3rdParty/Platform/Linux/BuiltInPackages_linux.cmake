#
# All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
# its licensors.
#
# For complete copyright and license terms please see the LICENSE at the root of this
# distribution (the "License"). All use of this software is governed by the License,
# or, if provided, by the license below or the license accompanying this file. Do not
# remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

# shared by other platforms:
ly_associate_package(PACKAGE_NAME zlib-1.2.8-rev2-multiplatform             TARGETS zlib        PACKAGE_HASH e6f34b8ac16acf881e3d666ef9fd0c1aee94c3f69283fb6524d35d6f858eebbb)
ly_associate_package(PACKAGE_NAME Lua-5.3.5-rev3-multiplatform              TARGETS Lua         PACKAGE_HASH 171dcdd60bd91fb325feaab0e53dd185c9d6e7b701d53e66fc6c2c6ee91d8bff)
ly_associate_package(PACKAGE_NAME ilmbase-2.3.0-rev4-multiplatform          TARGETS ilmbase     PACKAGE_HASH 97547fdf1fbc4d81b8ccf382261f8c25514ed3b3c4f8fd493f0a4fa873bba348)
ly_associate_package(PACKAGE_NAME hdf5-1.0.11-rev2-multiplatform            TARGETS hdf5        PACKAGE_HASH 11d5e04df8a93f8c52a5684a4cacbf0d9003056360983ce34f8d7b601082c6bd)
ly_associate_package(PACKAGE_NAME alembic-1.7.11-rev3-multiplatform         TARGETS alembic     PACKAGE_HASH ba7a7d4943dd752f5a662374f6c48b93493df1d8e2c5f6a8d101f3b50700dd25)
ly_associate_package(PACKAGE_NAME assimp-5.0.1-rev6-multiplatform           TARGETS assimplib   PACKAGE_HASH 47f1a6d05d101def036c030484c4a6e19d745aacd57037174715c7afe2b19b4c)
ly_associate_package(PACKAGE_NAME squish-ccr-20150601-rev3-multiplatform    TARGETS squish-ccr  PACKAGE_HASH c878c6c0c705e78403c397d03f5aa7bc87e5978298710e14d09c9daf951a83b3)
ly_associate_package(PACKAGE_NAME ASTCEncoder-2017_11_14-rev2-multiplatform TARGETS ASTCEncoder PACKAGE_HASH c240ffc12083ee39a5ce9dc241de44d116e513e1e3e4cc1d05305e7aa3bdc326)
ly_associate_package(PACKAGE_NAME md5-2.0-multiplatform                     TARGETS md5         PACKAGE_HASH 29e52ad22c78051551f78a40c2709594f0378762ae03b417adca3f4b700affdf)
ly_associate_package(PACKAGE_NAME RapidJSON-1.1.0-multiplatform             TARGETS RapidJSON   PACKAGE_HASH 18b0aef4e6e849389916ff6de6682ab9c591ebe15af6ea6017014453c1119ea1)
ly_associate_package(PACKAGE_NAME RapidXML-1.13-multiplatform               TARGETS RapidXML    PACKAGE_HASH 510b3c12f8872c54b34733e34f2f69dd21837feafa55bfefa445c98318d96ebf)
ly_associate_package(PACKAGE_NAME pybind11-2.4.3-rev2-multiplatform         TARGETS pybind11    PACKAGE_HASH d8012f907b6c54ac990b899a0788280857e7c93a9595405a28114b48c354eb1b)
ly_associate_package(PACKAGE_NAME cityhash-1.1-multiplatform                TARGETS cityhash    PACKAGE_HASH 0ace9e6f0b2438c5837510032d2d4109125845c0efd7d807f4561ec905512dd2)
ly_associate_package(PACKAGE_NAME lz4-r128-multiplatform                    TARGETS lz4         PACKAGE_HASH d7b1d5651191db2c339827ad24f669d9d37754143e9173abc986184532f57c9d)
ly_associate_package(PACKAGE_NAME zstd-1.35-multiplatform                   TARGETS zstd        PACKAGE_HASH 45d466c435f1095898578eedde85acf1fd27190e7ea99aeaa9acfd2f09e12665)
ly_associate_package(PACKAGE_NAME SQLite-3.32.2-rev3-multiplatform          TARGETS SQLite      PACKAGE_HASH dd4d3de6cbb4ce3d15fc504ba0ae0587e515dc89a25228037035fc0aef4831f4)
ly_associate_package(PACKAGE_NAME glad-2.0.0-beta-rev2-multiplatform        TARGETS glad        PACKAGE_HASH ff97ee9664e97d0854b52a3734c2289329d9f2b4cd69478df6d0ca1f1c9392ee)
ly_associate_package(PACKAGE_NAME xxhash-0.7.4-rev1-multiplatform           TARGETS xxhash      PACKAGE_HASH e81f3e6c4065975833996dd1fcffe46c3cf0f9e3a4207ec5f4a1b564ba75861e)
ly_associate_package(PACKAGE_NAME PVRTexTool-4.24.0-rev3-multiplatform      TARGETS PVRTexTool  PACKAGE_HASH d9c7e21e03b03cb560da4793a91a0d384ea621757cf87a1a82c83bfca6b23f97)

# platform-specific:
ly_associate_package(PACKAGE_NAME PhysX-4.1.0.25992954-rev1-linux   TARGETS PhysX           PACKAGE_HASH e3ca36106a8dbf1524709f8bb82d520920ebd3ff3a92672d382efff406c75ee3)
ly_associate_package(PACKAGE_NAME mikkelsen-1.0.0.4-linux           TARGETS mikkelsen       PACKAGE_HASH 5973b1e71a64633588eecdb5b5c06ca0081f7be97230f6ef64365cbda315b9c8)
ly_associate_package(PACKAGE_NAME googletest-1.8.1-rev4-linux       TARGETS googletest      PACKAGE_HASH 7b7ad330f369450c316a4c4592d17fbb4c14c731c95bd8f37757203e8c2bbc1b)
ly_associate_package(PACKAGE_NAME googlebenchmark-1.5.0-rev1-linux  TARGETS GoogleBenchmark PACKAGE_HASH e794530c60bd1ecb5f224a346091f23c8fdeb4cfd7ba70b5195d6d2e276447e3)
ly_associate_package(PACKAGE_NAME unwind-1.2.1-linux                TARGETS unwind          PACKAGE_HASH 3453265fb056e25432f611a61546a25f60388e315515ad39007b5925dd054a77)