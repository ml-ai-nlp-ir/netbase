package com.tinkerpop.blueprints;

import org.apache.commons.configuration.Configuration;
import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.MapConfiguration;
import org.apache.commons.configuration.PropertiesConfiguration;

import java.io.File;
import java.util.Map;

/**
 * Factory to construct new Graph instances from a Configuration object or properties file.
 *
 * @author Stephen Mallette (http://stephen.genoprime.com)
 */
public class GraphFactory {

    /**
     * Open a graph.  See each Graph instance for its configuration options.
     *
     * @param configuration A configuration object that specifies the minimally required properties for a Graph instance.
     *                      This minimum is determined by the Graph instance itself.
     * @return A Graph instance.
     */
    public static Graph open(final Configuration configuration) {
        final String clazz = configuration.getString("blueprints.graph", null);

        if (clazz == null) {
            throw new RuntimeException("Configuration must contain a valid 'blueprints.graph' setting");
        }

        Class graphClass;
        try {
            graphClass = Class.forName(clazz);
        } catch (ClassNotFoundException cnfe) {
            throw new RuntimeException(String.format("GraphFactory could not find [%s].  Ensure that the jar is in the classpath.", clazz));
        }

        Graph g;
        try {
            // directly instantiate if it is a Graph class otherwise try to use a factory
            if (Graph.class.isAssignableFrom(graphClass)) {
                g = (Graph) graphClass.getConstructor(Configuration.class).newInstance(configuration);
            } else {
                g = (Graph) graphClass.getMethod("open", Configuration.class).invoke(null, configuration);
            }
        } catch (NoSuchMethodException nsme) {
            throw new RuntimeException(String.format("GraphFactory can only instantiate Graph implementations that have a constructor with a single Apache Commons Configuration argument. [%s] does not seem to have one.", clazz));
        } catch (Exception ex) {
            throw new RuntimeException(String.format("GraphFactory could not instantiate this Graph implementation [%s].", clazz), ex);
        }

        return g;
    }

    /**
     * Open a graph.  See each Graph instance for its configuration options.
     *
     * @param configurationFile The location of a configuration file that specifies the minimally required properties
     *                          for a Graph instance. This minimum is determined by the Graph instance itself.
     * @return A Graph instance.
     */
    public static Graph open(final String configurationFile) {
        return open(getConfiguration(new File(configurationFile)));
    }

    /**
     * Open a graph. See each Graph instance for its configuration options.
     *
     * @param configuration A Map based configuration that will be converted to an Apache configuration object
     *                      via MapConfiguration and passed to the appropriate overload.
     *
     * @return A Graph instance.
     */
    public static Graph open(final Map configuration) {
        return open(new MapConfiguration(configuration));
    }

    private static final Configuration getConfiguration(final File dirOrFile) {
        if (dirOrFile == null) {
            throw new IllegalArgumentException("Need to specify a configuration file or storage directory");
        }

        if (!dirOrFile.isFile()) {
            throw new IllegalArgumentException("Location of configuration must be a file");
        }

        try {
            return new PropertiesConfiguration(dirOrFile);
        } catch (ConfigurationException e) {
            throw new IllegalArgumentException("Could not load configuration at: " + dirOrFile, e);
        }
    }
}
