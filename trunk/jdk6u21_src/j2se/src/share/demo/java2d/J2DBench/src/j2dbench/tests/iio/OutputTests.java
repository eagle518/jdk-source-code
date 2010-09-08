/*
 * @(#)OutputTests.java	1.2 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench.tests.iio;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import javax.imageio.ImageIO;
import javax.imageio.spi.IIORegistry;
import javax.imageio.spi.ImageOutputStreamSpi;
import javax.imageio.stream.FileCacheImageOutputStream;
import javax.imageio.stream.FileImageOutputStream;
import javax.imageio.stream.ImageOutputStream;
import javax.imageio.stream.MemoryCacheImageOutputStream;

import j2dbench.Group;
import j2dbench.Option;
import j2dbench.Result;
import j2dbench.TestEnvironment;

abstract class OutputTests extends IIOTests {

    protected static final int OUTPUT_FILE        = 1;
    protected static final int OUTPUT_ARRAY       = 2;
    protected static final int OUTPUT_FILECHANNEL = 3;

    protected static ImageOutputStreamSpi fileChannelIOSSpi;
    static {
        if (hasImageIO) {
            ImageIO.scanForPlugins();
            IIORegistry registry = IIORegistry.getDefaultInstance();
            java.util.Iterator spis =
                registry.getServiceProviders(ImageOutputStreamSpi.class,
                                             false);
            while (spis.hasNext()) {
                ImageOutputStreamSpi spi = (ImageOutputStreamSpi)spis.next();
                String klass = spi.getClass().getName();
                if (klass.endsWith("ChannelImageOutputStreamSpi")) {
                    fileChannelIOSSpi = spi;
                    break;
                }
            }
        }
    }

    protected static Group outputRoot;
    protected static Group outputOptRoot;

    protected static Group generalOptRoot;
    protected static Group.EnableSet generalDestRoot;
    protected static Option destFileOpt;
    protected static Option destByteArrayOpt;

    protected static Group imageioGeneralOptRoot;
    protected static Option destFileChannelOpt;
    protected static Option useCacheTog;

    public static void init() {
        outputRoot = new Group(iioRoot, "output", "Output Benchmarks");
        outputRoot.setTabbed();

        // Options
        outputOptRoot = new Group(outputRoot, "opts", "Options");

        // General Options
        generalOptRoot = new Group(outputOptRoot,
                                   "general", "General Options");
        generalDestRoot = new Group.EnableSet(generalOptRoot,
                                              "dest", "Destintations");
        destFileOpt = new OutputType("file", "File", OUTPUT_FILE);
        destByteArrayOpt = new OutputType("byteArray", "byte[]", OUTPUT_ARRAY);

        if (hasImageIO) {
            // Image I/O Options
            imageioGeneralOptRoot = new Group(outputOptRoot,
                                              "imageio", "Image I/O Options");
            if (fileChannelIOSSpi != null) {
                destFileChannelOpt =
                    new OutputType("fileChannel", "FileChannel",
                                   OUTPUT_FILECHANNEL);
            }
            useCacheTog = new Option.Toggle(imageioGeneralOptRoot, "useCache",
                                            "ImageIO.setUseCache()",
                                            Option.Toggle.Off);
        }

        OutputImageTests.init();
        if (hasImageIO) {
            OutputStreamTests.init();
        }
    }

    protected OutputTests(Group parent, String nodeName, String description) {
        super(parent, nodeName, description);
    }

    protected static class OutputType extends Option.Enable {
        private int type;

	public OutputType(String nodeName, String description, int type) {
	    super(generalDestRoot, nodeName, description, false);
            this.type = type;
        }

        public int getType() {
            return type;
        }

	public String getAbbreviatedModifierDescription(Object value) {
	    return getModifierValueName(value);
	}

	public String getModifierValueName(Object val) {
	    return getNodeName();
	}
    }

    protected static abstract class Context {
        int size;
        Object output;
        int outputType;
        OutputStream origStream;

        Context(TestEnvironment env, Result result) {
            size = env.getIntValue(sizeList);
            if (hasImageIO) {
                if (env.getModifier(useCacheTog) != null) {
                    ImageIO.setUseCache(env.isEnabled(useCacheTog));
                }
            }

            OutputType t = (OutputType)env.getModifier(generalDestRoot);
            outputType = t.getType();
        }

        void initOutput() {
            if ((outputType == OUTPUT_FILE) ||
                (outputType == OUTPUT_FILECHANNEL))
            {
                try {
                    File outputfile = File.createTempFile("iio", ".tmp");
                    outputfile.deleteOnExit();
                    output = outputfile;
                } catch (IOException e) {
                    System.err.println("error creating temp file");
                    e.printStackTrace();
                }
            }
        }

        ImageOutputStream createImageOutputStream() throws IOException {
            ImageOutputStream ios;
            switch (outputType) {
            case OUTPUT_FILE:
                ios = new FileImageOutputStream((File)output);
                break;
            case OUTPUT_ARRAY:
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                BufferedOutputStream bos = new BufferedOutputStream(baos);
                if (ImageIO.getUseCache()) {
                    ios = new FileCacheImageOutputStream(bos, null);
                } else {
                    ios = new MemoryCacheImageOutputStream(bos);
                }
                break;
            case OUTPUT_FILECHANNEL:
                FileOutputStream fos = new FileOutputStream((File)output);
                origStream = fos;
                java.nio.channels.FileChannel fc = fos.getChannel();
                ios = fileChannelIOSSpi.createOutputStreamInstance(fc, false,
                                                                   null);
                break;
            default:
                ios = null;
                break;
            }
            return ios;
        }

        void closeOriginalStream() throws IOException {
            if (origStream != null) {
                origStream.close();
                origStream = null;
            }
        }

        void cleanup(TestEnvironment env) {
        }
    }
}
